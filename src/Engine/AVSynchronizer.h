#pragma once

#include "Define/IAudioSamples.h"
#include "Define/IVideoFrame.h"
#include "Core/SyncNotifier.h"

#include <mutex>
#include <list>
#include <memory>
#include <thread>
#include <atomic>

namespace av {

class AVSynchronizer {
public:
    struct Listener {
        virtual void OnAVSynchronizerNotifyAudioSamples(std::shared_ptr<IAudioSamples>) = 0;
        virtual void OnAVSynchronizerNotifyVideoFrame(std::shared_ptr<IVideoFrame>) = 0;
        virtual void OnAVSynchronizerNotifyAudioFinished() = 0;
        virtual void OnAVSynchronizerNotifyVideoFinished() = 0;
        virtual ~Listener() = default;
    };

    void SetListener(Listener* listener);
    AVSynchronizer();
    ~AVSynchronizer();

    // 并发控制
    void Start();
    void Stop();
    void Reset();

private:
    // 以音频为基准进行同步
    void Synchronize();

    void ThreadLoop();
private:
    // 处理音频/视频流的结构体
    struct StreamInfo {
        float currentTimeStamp{0.0f};       // 当前处理到的位置(时间)
        bool isFinished{false};
        void Reset() {
            currentTimeStamp = 0.0f;
            isFinished = false;
        }
    };
    StreamInfo m_audioStreamInfo;
    StreamInfo m_videoStreamInfo;

    const double syncThreshold = 0.05;

    std::mutex m_mutex;
    std::list<std::shared_ptr<IAudioSamples>> m_auidoQueue;
    std::list<std::shared_ptr<IVideoFrame>> m_videoQueue;

    std::mutex m_listenerMutex;
    Listener* m_listener{nullptr};

    // 并发相关
    std::thread m_syncThread;
    std::atomic<bool> m_abort{false};
    std::atomic<bool> m_reset{false};
    SyncNotifier m_notifier;
};


}