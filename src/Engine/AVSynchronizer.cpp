#include "AVSynchronizer.h"

namespace av {

AVSynchronizer::AVSynchronizer() {
    Start();
}

AVSynchronizer::~AVSynchronizer() {
    Stop();
}

void AVSynchronizer::SetListener(Listener* listener) {
    m_listener = listener;
}

void AVSynchronizer::Start() {
    m_abort = false;
    // m_audioStreamInfo.Reset();
    // m_videoStreamInfo.Reset();
    m_audioStreamInfo = StreamInfo{};
    m_videoStreamInfo = StreamInfo{};
    m_syncThread = std::thread(&AVSynchronizer::ThreadLoop, this);
}

void AVSynchronizer::Stop() {
    m_abort = true;
    m_notifier.Notify();
    if (m_syncThread.joinable()) m_syncThread.join();
}

void AVSynchronizer::Reset() {
    m_reset = true;
    m_notifier.Notify();
}

void AVSynchronizer::ThreadLoop() {
    while (true) {
        m_notifier.Wait(100);
        if (m_abort) {
            break;
        }
        if (m_reset) {
            m_auidoQueue.clear();
            m_videoQueue.clear();
            m_reset = false;
        }
        Synchronize();
    }
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_videoQueue.clear();
    }
}


void AVSynchronizer::Synchronize() {
    std::unique_lock<std::mutex> lock(m_mutex);

    while (!m_auidoQueue.empty()) {
        auto audioSamples = m_auidoQueue.front();
        // 当音频帧处理完毕，表示处理结束
        if (audioSamples->flags & AVFrameFlag::kEOS) {
            m_audioStreamInfo.isFinished = true;
            m_auidoQueue.pop_front();
            std::lock_guard<std::mutex> listenerLock(m_listenerMutex);
            if (m_listener) {
                m_listener->OnAVSynchronizerNotifyAudioFinished();
            } 
        } else if (audioSamples->flags & AVFrameFlag::kFlush) {
            m_auidoQueue.clear();
            m_videoQueue.clear();
        } else {
            // 处理音频，即直接发送给播放器进行播放
            m_audioStreamInfo.currentTimeStamp = audioSamples->GetTimeStamp();
            m_auidoQueue.pop_front();
            std::lock_guard<std::mutex> listenerLock(m_listenerMutex);
            if (m_listener) {
                m_listener->OnAVSynchronizerNotifyAudioSamples(audioSamples);
            }
        }
    }

    while (!m_videoQueue.empty()) {
        auto videoFrame = m_videoQueue.front();
        if (videoFrame->flags & AVFrameFlag::kEOS) {
            m_videoStreamInfo.isFinished = true;
            m_videoQueue.pop_front();
            std::lock_guard<std::mutex> listenerLock(m_listenerMutex);
            if (m_listener) {
                m_listener->OnAVSynchronizerNotifyVideoFinished();
            }
            continue;
        } else if (videoFrame->flags & AVFrameFlag::kFlush) {
            m_auidoQueue.clear();
            m_videoQueue.clear();
            continue;
        }
        // 视频帧处理，三种情况：
        // 1. 视频落后太多，丢弃该帧
        // 2. 视频超前太多，停止处理
        // 3. 交给播放器进行播放
        auto timeDiff = m_audioStreamInfo.currentTimeStamp - videoFrame->GetTimeStamp();
        if (timeDiff > syncThreshold) {
            m_videoStreamInfo.currentTimeStamp = videoFrame->GetTimeStamp();
            m_videoQueue.pop_front();
            std::lock_guard<std::mutex> listenerLock(m_listenerMutex);
            if (m_listener) {
                m_listener->OnAVSynchronizerNotifyVideoFrame(videoFrame);
            } 
            // 处理下一帧
            continue;
        } else if (timeDiff < -syncThreshold) {
            // 视频超前，不处理视频，直接跳过
            break;
        } else {
            m_videoStreamInfo.currentTimeStamp = videoFrame->GetTimeStamp();
            m_videoQueue.pop_front();
            std::lock_guard<std::mutex> listenerLock(m_listenerMutex);
            if (m_listener) {
                m_listener->OnAVSynchronizerNotifyVideoFrame(videoFrame);
            }
            // 处理结束
            break;
        }
    }
}
}