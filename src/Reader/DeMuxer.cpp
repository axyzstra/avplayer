#include "DeMuxer.h"

namespace av {

IDeMuxer* IDeMuxer::Create() {
    return new DeMuxer();
}

void DeMuxer::ReleaseVideoPipelineResource() {
    m_audioStream.pipelineResourceCount++;
}

void DeMuxer::ReleaseAudioPipelineResource() {
    m_videoStream.pipelineResourceCount++;
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
        } else if (m_formatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            m_videoStream.streamIndex = i;
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
    int ret = av_read_frame(m_formatCtx, &packet);
    if (ret >= 0) {
        std::lock_guard<std::recursive_mutex> lock(m_listenerMutex);
        if (m_listener) {
            if (packet.stream_index == m_audioStream.streamIndex) {
                m_audioStream.pipelineResourceCount--;
                auto sharedPacket = std::make_shared<IAVPacket>(av_packet_clone(&packet));
                sharedPacket->releaseCallback = m_audioStream.pipelineReleaseCallback;
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

}