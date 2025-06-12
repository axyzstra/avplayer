#pragma once

#include "Interface/IDeMuxer.h"
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
    ~DeMuxer();


    void SetListener(IDeMuxer::Listener* listener) override;


    /// @brief 打开文件，并读取流到当前类中
    /// @return 
    bool Open(const std::string& url) override;


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


    /// @brief 从 stream 中读取 packet, 并通过 listener 通知上层
    /// @return 
    bool ReadAndSendPacket();

    void ProcessSeek();

private:
    IDeMuxer::Listener* m_listener{nullptr};

    // 同步互斥
    std::thread m_thread;
    std::mutex m_formatMutex;   // 管理 m_formatCtx
    std::recursive_mutex m_listenerMutex;   // 管理 listener

    std::atomic<bool> m_paused;
    std::atomic<bool> m_seek{false};


    // 音视频操作相关
    AVFormatContext* m_formatCtx{nullptr};
    StreamInfo m_audioStream;
    StreamInfo m_videoStream;
    float m_seekProgress{-1.0f};

};

}