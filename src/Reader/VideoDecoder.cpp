#include "VideoDecoder.h"

namespace av {

IVideoDecoder* IVideoDecoder::Create() {
    return new VideoDecoder();
}


VideoDecoder::VideoDecoder() {
    m_pipelineReleaseCallback = std::make_shared<std::function<void()>>([&](){
        ReleaseVideoPipelineResource();
    });
    m_thread = std::thread(&VideoDecoder::ThreadLoop, this);
}

VideoDecoder::~VideoDecoder() {
    Stop();
    std::lock_guard<std::mutex> lock(m_codecContextMutex);
    CleanContext();
}

void VideoDecoder::CleanContext() {
    if (m_codecContext) {
        avcodec_free_context(&m_codecContext);
    }
    if (m_swsContext) {
        sws_freeContext(m_swsContext);
    }
}

void VideoDecoder::ThreadLoop() {
    while (true) {
        m_notifier.Wait(100);
        if (m_abort) {
            break;
        }
        CheckFlushPacket();
        if (!m_paused && m_pipelineResourceCount > 0) {
            DecodeAVPacket();
        }
    }
    {
        std::lock_guard<std::mutex> lock(m_packetQueueMutex);
        m_packetQueue.clear();
    }
}

void VideoDecoder::Start() {
    m_paused = false;
    m_notifier.Notify();
}

void VideoDecoder::Pause() {
    m_paused = true;
}

void VideoDecoder::Stop() {
    m_abort = true;
    m_notifier.Notify();
    if (m_thread.joinable()) {
        m_thread.join();
    }
}

int VideoDecoder::GetVideoHeight() {
    std::lock_guard<std::mutex> lock(m_codecContextMutex);
    return m_codecContext ? m_codecContext->width : 0;
}

int VideoDecoder::GetVideoWidth() {
    std::lock_guard<std::mutex> lock(m_codecContextMutex);
    return m_codecContext ? m_codecContext->height : 0;
}


void VideoDecoder::CheckFlushPacket() {
    std::lock_guard<std::mutex> lock(m_packetQueueMutex);
    if (m_packetQueue.empty()) return;

    auto packet = m_packetQueue.front();
    if (packet->flags & AVFrameFlag::kFlush) {
        m_packetQueue.pop_front();
        avcodec_flush_buffers(m_codecContext);

        auto videoFrame = std::make_shared<IVideoFrame>();
        videoFrame->flags |= AVFrameFlag::kFlush;
        std::lock_guard<std::recursive_mutex> lock(m_listenerMutex);
        if (m_listener) m_listener->OnNotifyVideoFrame(videoFrame);
    }
}

void VideoDecoder::SetStream(struct AVStream* stream) {
    if (stream == nullptr) {
        return;
    }
    {
        std::lock_guard<std::mutex> lock(m_codecContextMutex);
        m_packetQueue.clear();
    }
    // 重置 code 和 sws 的 ctx
    std::lock_guard<std::mutex> lock(m_codecContextMutex);
    CleanContext();

    // 获取一致的解码器
    AVCodec* codec = avcodec_find_decoder(stream->codecpar->codec_id);
    if (!codec) {
        return;
    }

    m_codecContext = avcodec_alloc_context3(codec);
    if (!m_codecContext) {
        return;
    }

    // 拷贝视频流解码器参数
    if (avcodec_parameters_to_context(m_codecContext, stream->codecpar) < 0) {
        avcodec_free_context(&m_codecContext);
        return;
    }
    // 打开解码器
    if (avcodec_open2(m_codecContext, codec, nullptr) < 0) {
        avcodec_free_context(&m_codecContext);
        return;
    }

    m_timeBase = stream->time_base;
}

void VideoDecoder::SetListener(IVideoDecoder::Listener* listener) {
    std::lock_guard<std::recursive_mutex> lock(m_listenerMutex);
    m_listener = listener;
}


void VideoDecoder::DecodeAVPacket() {
    std::shared_ptr<IAVPacket> packet;
    {
        std::lock_guard<std::mutex> lock(m_packetQueueMutex);
        if (m_packetQueue.empty()) {
            return;
        }
        packet = m_packetQueue.front();
        m_packetQueue.pop_front();
    }
    std::lock_guard<std::mutex> lock(m_codecContextMutex);
    if (packet->avPacket && avcodec_send_packet(m_codecContext, packet->avPacket) < 0) {
        std::cerr << "Error sending video packet for decoding." << std::endl;
        return;
    }
    AVFrame* frame = av_frame_alloc();
    if (!frame) {
        std::cerr << "Could not allocate video frame." << std::endl;
        return;
    }


    while (true) {
        int ret = avcodec_receive_frame(m_codecContext, frame);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            break;
        } else if (ret < 0) {
            std::cerr << "Error during decoding." << std::endl;
            av_frame_free(&frame);
            return;
        }

        // 创建一个图像转换器，用于定义图像缩放和格式转换的参数。
        // 转换为 AV_PIX_FMT_RGBA 格式，使用 SWS_BILINEAR(双线性插值)
        if (!m_swsContext) {
            m_swsContext = sws_getContext(frame->width, frame->height, (AVPixelFormat)frame->format, frame->width,
                            frame->height, AV_PIX_FMT_RGBA, SWS_BILINEAR, nullptr, nullptr, nullptr);
            if (!m_swsContext) {
                av_frame_free(&frame);
                return;
            }
        }

        AVFrame* rgbFrame = av_frame_alloc();
        if (!rgbFrame) {
            av_frame_free(&frame);
            return;
        }

        // 计算缓冲区大小
        int numBytes = av_image_get_buffer_size(AV_PIX_FMT_RGBA, frame->width, frame->height, 1);
        std::shared_ptr<uint8_t> buffer(new uint8_t[numBytes], std::default_delete<uint8_t[]>());

        // 这个函数只是指明了 rgbFrame->data 指针应该指向的区域是 buffer，对该区域进行访问时的步长为：rgbFrame->linesize, 对数据实际是不做处理的
        if (av_image_fill_arrays(rgbFrame->data, rgbFrame->linesize, buffer.get(), AV_PIX_FMT_RGBA, frame->width, frame->height, 1) < 0) {
            av_frame_free(&frame);
            av_frame_free(&rgbFrame);
            return;
        }

        // 将图像转换为目标格式 frame->rgbframe
        sws_scale(m_swsContext, frame->data, frame->linesize, 0, frame->height, rgbFrame->data, rgbFrame->linesize);
        auto videoFrame = std::make_shared<IVideoFrame>();
        videoFrame->width = frame->width;
        videoFrame->height = frame->height;
        videoFrame->data = std::move(buffer);
        videoFrame->pts = frame->pts;
        videoFrame->duration = frame->pkt_duration;
        videoFrame->timebaseNum = m_timeBase.num;
        videoFrame->timebaseDen = m_timeBase.den;
        videoFrame->releaseCallback = m_pipelineReleaseCallback;
        av_frame_free(&rgbFrame);

        m_pipelineResourceCount--;
        {
            std::lock_guard<std::recursive_mutex> lock(m_listenerMutex);
            if (m_listener) {
                m_listener->OnNotifyVideoFrame(videoFrame);
            }
        }
    }
    av_frame_free(&frame);
}

void VideoDecoder::Decode(std::shared_ptr<IAVPacket> packet) {
    if (packet == nullptr) {
        return;
    }
    std::lock_guard<std::mutex> lock(m_packetQueueMutex);
    if (packet->flags & AVFrameFlag::kFlush) {
        m_packetQueue.clear();
    }
    m_packetQueue.push_back(packet);
    m_notifier.Notify();
}


void VideoDecoder::ReleaseVideoPipelineResource() {
    m_pipelineResourceCount++;
    m_notifier.Notify();
}

}