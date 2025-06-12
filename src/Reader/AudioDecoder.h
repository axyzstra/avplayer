#pragma once

#include "Interface/IAudioDecoder.h"
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/opt.h>
#include <libswresample/swresample.h>
}
#include <mutex>
#include <list>
#include <memory>
#include <iostream>

namespace av {

class AudioDecoder : public IAudioDecoder {
public:
    AudioDecoder(unsigned int channels, unsigned int sampleRate);
    ~AudioDecoder() override;

    void SetStream(struct AVStream* stream) override;
    void SetListener(IAudioDecoder::Listener* listener) override;
    void Decode(std::shared_ptr<IAVPacket> packet) override;

private:
    // bool DecodeAVPacket(AVPacket* packet);
    // 对 que 中的 packet 进行解码
    void DecodeAVPacket();
    void CleanupContext();

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

    // 释放资源的回调函数
    std::shared_ptr<std::function<void()>> m_pipelineReleaseCallback;

    // 对音视频处理管线中的资源流量控制和同步。
    std::atomic<int> m_pipelineReleaseCount{3};
};

}