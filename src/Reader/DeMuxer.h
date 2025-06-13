#pragma once

#include "Interface/IDeMuxer.h"
#include "Core/SyncNotifier.h"
#include <mutex>
#include <atomic>
#include <memory>
#include <thread>
#include <functional>
#include <iostream>

extern "C" {
#include <libavformat/avformat.h>
}

namespace av {

class DeMuxer : public IDeMuxer {
public:
    DeMuxer();
    // 析构函数最好加上 override
    ~DeMuxer() override;

    // 线程控制
    void Start() override;
    void Pause() override;
    void Stop() override;

    void SetListener(IDeMuxer::Listener* listener) override;

    /// @brief 打开文件，并读取流到当前类中
    /// @return 
    bool Open(const std::string& url) override;
    
    // 跳转到指定进度
    void SeekTo(float progress) override;

    // 获取总时长
    float GetDuration() override;

private:
    struct StreamInfo {
        // 当前流下标
        int streamIndex{-1};
        // 
        std::atomic<int> pipelineResourceCount{3};
        std::shared_ptr<std::function<void()>> pipelineReleaseCallback;
    };
    void ReleaseVideoPipelineResource();
    void ReleaseAudioPipelineResource();

    // 解复用线程，不断循环，执行解复用
    void ThreadLoop();


    /// @brief 从 stream 中读取 packet, 并通过 listener 通知上层
    /// @return 
    bool ReadAndSendPacket();
    // 跳转到指定时间戳然后进行解复用
    void ProcessSeek();

private:
    IDeMuxer::Listener* m_listener{nullptr};
    std::recursive_mutex m_listenerMutex;   // 管理 listener

    // 并发控制相关
    std::thread m_thread;
    SyncNotifier m_notifier;
    std::atomic<bool> m_paused{true};
    std::atomic<bool> m_abort{false};


    // 音视频操作相关
    std::mutex m_formatMutex;   // 管理 m_formatCtx
    AVFormatContext* m_formatCtx{nullptr};
    StreamInfo m_audioStream;
    StreamInfo m_videoStream;
    // 跳转
    std::atomic<bool> m_seek{false};
    float m_seekProgress{-1.0f};
};

}