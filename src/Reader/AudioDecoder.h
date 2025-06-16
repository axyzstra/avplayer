#pragma once

#include "Interface/IAudioDecoder.h"
#include "Core/SyncNotifier.h"
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/opt.h>
#include <libswresample/swresample.h>
#include <libavutil/channel_layout.h>
}
#include <mutex>
#include <list>
#include <memory>
#include <iostream>
#include <thread>

namespace av {

class AudioDecoder : public IAudioDecoder {
public:
    AudioDecoder(unsigned int channels, unsigned int sampleRate);
    ~AudioDecoder() override;

    void SetStream(struct AVStream* stream) override;
    void SetListener(IAudioDecoder::Listener* listener) override;
    // 将 packet 放入待解码的队列
    void Decode(std::shared_ptr<IAVPacket> packet) override;

    // 线程相关
    void Start() override;
    void Pause() override;
    void Stop() override;

private:
    // bool DecodeAVPacket(AVPacket* packet);
    // 对 que 中的 packet 进行解码
    void DecodeAVPacket();

    // 刷新包
    void CheckFlushPacket();
    void CleanupContext();

    void ReleaseAudioPipelineResource();

    // 线程相关
    void ThreadLoop();

private:
    unsigned int m_taragetChannels;
    unsigned int m_taragetSampleRate;

    // 监听器，监听状态，发送消息
    IAudioDecoder::Listener* m_listener{nullptr};
    std::recursive_mutex m_listenerMutex;

    // packet 包队列
    std::mutex m_packetQueueMutex;
    std::list<std::shared_ptr<IAVPacket>> m_packetQueue;

    // 解码
    std::mutex m_codecContextMutex;
    AVCodecContext* m_codecContext{nullptr};

    // 重采样
    SwrContext* m_swrContext{nullptr};
    AVRational m_timeBase{AVRational{1, 1}};

    // 线程控制
    std::thread m_thread;
    std::atomic<bool> m_paused{true};
    std::atomic<bool> m_abort{false};
    SyncNotifier m_notifier;

    // 释放资源的回调函数
    std::shared_ptr<std::function<void()>> m_pipelineReleaseCallback;
    // 对音视频处理管线中的资源流量控制和同步。
    std::atomic<int> m_pipelineResourceCount{3};
};

}