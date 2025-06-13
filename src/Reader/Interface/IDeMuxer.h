#pragma once
#include <string>

#include "Define/IAVPacket.h"

namespace av {
struct IDeMuxer {
    // 监听器，用于监听 DeMuxer 状态
    struct Listener {

        // 解复用后通知上层能够开始处理 packet
        virtual void OnNotifyAudioPacket(std::shared_ptr<IAVPacket> packet) = 0;
        virtual void OnNotifyVideoPacket(std::shared_ptr<IAVPacket> packet) = 0;

        // 当解复用结束进行通知
        virtual void OnNotifyAudioStream(struct AVStream* stream) = 0;
        virtual void OnNotifyVideoStream(struct AVStream* stream) = 0;

        virtual ~Listener() = default;
    };


    virtual ~IDeMuxer() = default;
    static IDeMuxer* Create();

    virtual void SetListener(Listener* listener) = 0;
    virtual bool Open(const std::string& url);
    virtual void SeekTo(float progress)= 0;
    virtual float GetDuration() = 0;

    virtual void Start() = 0;
    virtual void Pause() = 0;
    virtual void Stop() = 0;
};
}