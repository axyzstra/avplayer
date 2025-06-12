#pragma once

#include <functional>
#include <memory>

#include "BaseDef.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/rational.h>
}

struct AVPacket;

namespace av {

struct IAVPacket {
    int flags{0};
    struct AVPacket* avPacket{nullptr};
    AVRational timeBase{AVRational{0, 0}};
    std::weak_ptr<std::function<void()>> releaseCallback;

    explicit IAVPacket(struct AVPacket* avPacket) : avPacket(avPacket) {}
    virtual ~IAVPacket() {
        if (avPacket) av_packet_free(&avPacket);
        if (auto lockedPtr = releaseCallback.lock()) {
            (*lockedPtr)();
        }
    }

    // 计算并返回数据包的时间戳（以秒为单位）
    float GetTimeStamp() const {
        return avPacket ? avPacket->pts * 1.0f * timeBase.num / timeBase.den : -1.0f;
    }
};

}
