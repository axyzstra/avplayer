#include "DeMuxer.h"

namespace av {

IDeMuxer* IDeMuxer::Create() {
    return new DeMuxer();
}

void DeMuxer::ReleaseVideoPipelineResource() {
    m_audioStream.pipelineResourceCount++;
    m_notifier.Notify();
}

void DeMuxer::ReleaseAudioPipelineResource() {
    m_videoStream.pipelineResourceCount++;
    m_notifier.Notify();
}

void DeMuxer::SetListener(IDeMuxer::Listener* listener) {
    std::lock_guard<std::recursive_mutex> lock(m_listenerMutex);
    m_listener = listener;
}

DeMuxer::DeMuxer() {
    m_videoStream.pipelineReleaseCallback = std::make_shared<std::function<void()>>([&]() {
        ReleaseVideoPipelineResource();
    });
    m_audioStream.pipelineReleaseCallback = std::make_shared<std::function<void()>>([&]() {
        ReleaseAudioPipelineResource();
    });
    m_thread = std::thread(&DeMuxer::ThreadLoop, this);
}

DeMuxer::~DeMuxer() {
    Stop();
    if (m_thread.joinable()) {
        m_thread.join();
    }
    std::lock_guard<std::mutex> lock(m_formatMutex);
    if (m_formatCtx) {
        avformat_close_input(&m_formatCtx);
    }
}

bool DeMuxer::Open(const std::string& url) {
    std::lock_guard<std::mutex> lock(m_formatMutex);
    // 打开文件
    if (avformat_open_input(&m_formatCtx, url.c_str(), nullptr, nullptr) != 0) {
        return false;
    }
    // 读取流
    if (avformat_find_stream_info(m_formatCtx, nullptr) < 0) {
        return false;
    }
    for (unsigned int i = 0; i < m_formatCtx->nb_streams; i++) {
        if (m_formatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            m_audioStream.streamIndex = i;
            std::lock_guard<std::recursive_mutex> lock(m_listenerMutex);
            m_listener->OnNotifyAudioStream(m_formatCtx->streams[i]);
        } else if (m_formatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            m_videoStream.streamIndex = i;
            std::lock_guard<std::recursive_mutex> lock(m_listenerMutex);
            m_listener->OnNotifyVideoStream(m_formatCtx->streams[i]);
        }
    }
    return true;
}

bool DeMuxer::ReadAndSendPacket() {
    std::lock_guard<std::mutex> lock(m_formatMutex);
    if (m_formatCtx == nullptr) {
        m_paused = true;
        return true;
    }

    AVPacket packet;
    // 从 ctx 中读取数据放入 packet
    int ret = av_read_frame(m_formatCtx, &packet);
    if (ret >= 0) {
        // 对 packet 进行处理
        std::lock_guard<std::recursive_mutex> lock(m_listenerMutex);
        if (m_listener) {
            if (packet.stream_index == m_audioStream.streamIndex) {
                m_audioStream.pipelineResourceCount--;
                auto sharedPacket = std::make_shared<IAVPacket>(av_packet_clone(&packet));
                sharedPacket->releaseCallback = m_audioStream.pipelineReleaseCallback;
                // 通知解码器进行解码
                m_listener->OnNotifyAudioPacket(sharedPacket);
            } else if (packet.stream_index == m_videoStream.streamIndex) {
                m_videoStream.pipelineResourceCount--;
                auto sharedPacket = std::make_shared<IAVPacket>(av_packet_clone(&packet));
                sharedPacket->releaseCallback = m_videoStream.pipelineReleaseCallback;
                m_listener->OnNotifyVideoPacket(sharedPacket);
            }
        }
        av_packet_unref(&packet);
    } else if (ret == AVERROR_EOF) {
        m_paused = true;
    } else {
        return false;
    }
    return true;
}

float DeMuxer::GetDuration() {
    std::lock_guard<std::mutex> lock(m_formatMutex);
    if (m_formatCtx == nullptr) {
        return 0;
    }
    return m_formatCtx->duration / static_cast<float>(AV_TIME_BASE);
}


void DeMuxer::ProcessSeek() {
    std::lock_guard<std::mutex> lock(m_formatMutex);
    if (m_formatCtx == nullptr) {
        return;
    }
    int64_t timestamp = static_cast<int64_t>(m_seekProgress * m_formatCtx->duration);
    // 跳转到指定的时间戳 timestamp
    if (av_seek_frame(m_formatCtx, -1, timestamp, AVSEEK_FLAG_BACKWARD) < 0) {
        std::cerr << "Seek failed" << std::endl;
    }

    // 跳转到指定位置后进行解复用，然后通知解码器处理 packet
    {
        std::lock_guard<std::recursive_mutex> listenerLock(m_listenerMutex);
        auto audioPacket = std::make_shared<IAVPacket>(nullptr);
        audioPacket->flags |= AVFrameFlag::kFlush;
        m_listener->OnNotifyAudioPacket(audioPacket);

        auto videoPacket = std::make_shared<IAVPacket>(nullptr);
        videoPacket->flags |= AVFrameFlag::kFlush;
        m_listener->OnNotifyVideoPacket(videoPacket);
    }
    m_seekProgress = -1.0f;
    m_seek = false;
}

void DeMuxer::SeekTo(float progress) {
    m_seekProgress = progress;
    m_seek = true;
    m_notifier.Notify();
}

void DeMuxer::ThreadLoop() {
    while(true) {
        // 限时阻塞，主要是避免解复用太快
        m_notifier.Wait(100);
        if (m_abort) {
            break;;
        }
        if (m_seek) {
            ProcessSeek();
        }
        // 当线程未暂停且流缓冲充足则进行解复用 (count 用于控制解复用速度，避免后面解码难以跟上)
        if (!m_paused && (m_audioStream.pipelineResourceCount > 0 || m_videoStream.pipelineResourceCount > 0)) {
            if (!ReadAndSendPacket()) {
                break;
            }
        }
    }
}


void DeMuxer::Start() {
    m_paused = false;
    m_notifier.Notify();
}

void DeMuxer::Pause() {
    m_paused = true;
}

void DeMuxer::Stop() {
    m_abort = true;
    m_notifier.Notify();
}

}