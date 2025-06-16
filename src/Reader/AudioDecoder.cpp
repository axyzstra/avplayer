#include "AudioDecoder.h"

namespace av {

AudioDecoder::AudioDecoder(unsigned int channels, unsigned int sampleRate) : 
    m_taragetChannels(channels), m_taragetSampleRate(sampleRate) {

        m_pipelineReleaseCallback = std::make_shared<std::function<void()>>([&]() {
            ReleaseAudioPipelineResource();
        });
        m_thread = std::thread(&AudioDecoder::ThreadLoop, this);
}

AudioDecoder::~AudioDecoder() {
    Stop();
    std::lock_guard<std::mutex> lock(m_codecContextMutex);
    CleanupContext();
}

void AudioDecoder::Start() {
    m_paused = false;
    m_notifier.Notify();
}

void AudioDecoder::Pause() {
    m_paused = true;
}

void AudioDecoder::Stop() {
    m_abort = true;
    m_notifier.Notify();
    if (m_thread.joinable()) {
        m_thread.join();
    }
}

void AudioDecoder::ThreadLoop() {
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
    // 线程结束时清空 packet 队列
    {
        std::lock_guard<std::mutex> lock(m_packetQueueMutex);
        m_packetQueue.clear();
    }
}

void AudioDecoder::CheckFlushPacket() {
    std::lock_guard<std::mutex> lock(m_packetQueueMutex);
    if (m_packetQueue.empty()) {
        return;
    }
    auto packet = m_packetQueue.front();
    if (packet->flags & AVFrameFlag::kFlush) {
        m_packetQueue.pop_front();
        avcodec_flush_buffers(m_codecContext);

        auto audioSamples = std::make_shared<IAudioSamples>();
        audioSamples->flags |= AVFrameFlag::kFlush;
        std::lock_guard<std::recursive_mutex> lock(m_listenerMutex);
        if (m_listener) m_listener->OnNotifyAudioSamples(audioSamples);
    }
}

IAudioDecoder* IAudioDecoder::Create(unsigned int channels, unsigned int sampleRate) {
    return new AudioDecoder(channels, sampleRate);
}

void AudioDecoder::SetListener(IAudioDecoder::Listener* listener) {
    std::lock_guard<std::recursive_mutex> lock(m_listenerMutex);
    m_listener = listener;
}

void AudioDecoder::SetStream(AVStream* stream) {
    if (stream == nullptr) {
        return;
    }
    {
        // 清空包队列
        std::lock_guard<std::mutex> lock(m_packetQueueMutex);
        m_packetQueue.clear();
    }

    std::lock_guard<std::mutex> lock(m_codecContextMutex);
    CleanupContext();

    // m_codecContext = avcodec_alloc_context3(nullptr);
    // if (avcodec_parameters_to_context(m_codecContext, stream->codecpar) < 0) {
    //     std::cerr << "ailed to copy codec parameters to decoder conte" << std::endl;
    //     return;
    // }

    // // 定义对应的解码器
    // AVCodec* codec = avcodec_find_decoder(m_codecContext->codec_id);
    // if (!codec) {
    //     std::cerr << "Codec not found." << std::endl;
    //     return;
    // }

    // if (avcodec_open2(m_codecContext, codec, nullptr) < 0) {
    //     std::cerr << "Failed to open codec." << std::endl;
    //     return;
    // }


    // 获取解码器参数
    AVCodecParameters* codec_params = stream->codecpar;
    // 定义对应的解码器
    const AVCodec* codec = avcodec_find_decoder(codec_params->codec_id);
    if (!codec) {
        std::cerr << "Codec not found." << std::endl;
        return;
    }
    // 分配解码器 ctx
    m_codecContext = avcodec_alloc_context3(codec);
    // 设置解码器参数
    if (avcodec_parameters_to_context(m_codecContext, stream->codecpar) < 0) {
        std::cerr << "ailed to copy codec parameters to decoder conte" << std::endl;
        return;
    }
    // 打开解码器
    if (avcodec_open2(m_codecContext, codec, nullptr) < 0) {
        std::cerr << "Failed to open codec." << std::endl;
        return;
    }


    // 重采样设置
    m_swrContext = swr_alloc_set_opts(nullptr, av_get_default_channel_layout(m_taragetChannels), AV_SAMPLE_FMT_S16,
                                    m_taragetSampleRate, av_get_default_channel_layout(m_codecContext->channels),
                                    m_codecContext->sample_fmt, m_codecContext->sample_rate, 0, nullptr);
    if (!m_swrContext || swr_init(m_swrContext) < 0) {
        std::cerr << "Failed to initialize resampler" << std::endl;
        return;
    }
    m_timeBase = stream->time_base; 
}


// ======================核心实现========================================
// bool AudioDecoder::DecodeAVPacket(AVPacket* packet) {
//     // 将数据 packet 发送到解码器
//     if (avcodec_send_packet(m_codecContext, packet) < 0) {
//         std::cerr << "Error sending packet to decoder" << std::endl;
//         return false;
//     }
//     // 处理解码器中的数据
//     while (true) {
//         AVFrame* frame = av_frame_alloc();
//         int ret = avcodec_receive_frame(m_codecContext, frame);
//         if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
//             av_frame_free(&frame);
//             break;
//         } else if (ret < 0) {
//             std::cerr << "Error receiving frame from decoder." << std::endl;
//             av_frame_free(&frame);
//             return false;
//         }

//         // 重采样
//         uint8_t* outputBuffer = nullptr;
//         int outSamples = swr_convert(
//             m_swrContext,
//             &outputBuffer,
//             frame->nb_samples,
//             (const uint8_t**)frame->data,
//             frame->nb_samples
//         );

//         if (outSamples < 0) {
//             std::cerr << "Erro during resampling." << std::endl;
//         }

//         av_frame_free(&frame);
//     }
//     return true;
// }
// ========================================================================


void AudioDecoder::DecodeAVPacket() {
    // 取出队列中的 packet
    std::shared_ptr<IAVPacket> packet;
    {
        std::lock_guard<std::mutex> lock(m_packetQueueMutex);
        if (m_packetQueue.empty()) {
            return;
        }
        packet = m_packetQueue.front();
        m_packetQueue.pop_front();
    }
    // 将 packet 放入解码器
    if (packet->avPacket && avcodec_send_packet(m_codecContext, packet->avPacket) < 0) {
        std::cerr << "Error sending audio packet for decoding." << std::endl;
        return;
    }

    // 定义 frame 存放解码后的数据
    AVFrame* frame = av_frame_alloc();
    if (!frame) {
        std::cerr << "allocate audio frame fail." << std::endl;
        return;
    }

    // 对解码器中的数据进行解码，放到 frame 直到解码器为空
    while (avcodec_receive_frame(m_codecContext, frame) >= 0) {
        // 重采样后的样本数量
        int dst_nb_samples = 
            av_rescale_rnd(swr_get_delay(m_swrContext, m_codecContext->sample_rate) + frame->nb_samples,
            m_taragetSampleRate, m_codecContext->sample_rate, AV_ROUND_UP);
        
        // 要存储固定样本数量和通道所需的空间
        int buffer_size = av_samples_get_buffer_size(nullptr, m_taragetChannels, dst_nb_samples, AV_SAMPLE_FMT_S16, 1);
        // 分配空间
        uint8_t* buffer = (uint8_t*)av_malloc(buffer_size);

        // 进行重采样
        int ret = swr_convert(m_swrContext, &buffer, dst_nb_samples, (const uint8_t**)frame->data, frame->nb_samples);
        if (ret < 0) {
            std::cerr << "Error while converting." << std::endl;
            av_free(buffer);
            av_frame_free(&frame);
            return;
        }

        // 将原始数据 frame，即 PCM 进行封装
        std::shared_ptr<IAudioSamples> samples = std::make_shared<IAudioSamples>();
        samples->channels = m_taragetChannels;
        samples->sampleRate = m_taragetSampleRate;
        samples->pts = frame->pts;
        samples->duration = frame->pkt_duration;
        samples->timebaseNum = m_timeBase.num;
        samples->timebaseDen = m_timeBase.den;
        samples->pcmData.assign((int16_t*)buffer, (int16_t*)(buffer + buffer_size));
        samples->releaseCallback = m_pipelineReleaseCallback;
        av_free(buffer);

        m_pipelineResourceCount--;

        // 监听器发送消息通知原始数据准备完毕
        std::lock_guard<std::recursive_mutex> lock(m_listenerMutex);
        if (m_listener) {
            m_listener->OnNotifyAudioSamples(samples);
        }
    }
}


void AudioDecoder::Decode(std::shared_ptr<IAVPacket> packet) {
    if (packet == nullptr) {
        return;
    }
    std::lock_guard<std::mutex> lock(m_packetQueueMutex);
    if (packet->flags & AVFrameFlag::kFlush) {
        m_packetQueue.clear();
    }
    // m_packetQueue.push_back(packet);
    m_packetQueue.emplace_back(packet);
    m_notifier.Notify();
}

void AudioDecoder::ReleaseAudioPipelineResource() {
    m_pipelineResourceCount++;
    m_notifier.Notify();
}

void AudioDecoder::CleanupContext() {
    if (m_codecContext) {
        avcodec_free_context(&m_codecContext);
    }
    if (m_swrContext) {
        swr_free(&m_swrContext);
    }
}

}