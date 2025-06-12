#pragma once

#include <cstdint>
#include <memory>
#include <functional>

namespace av {
// 封装和管理一个视频帧及其相关元数据
struct IVideoFrame {
    int flags{0};
    unsigned int width{0};
    unsigned int height{0};
    int64_t pts{0};
    int64_t duration{0};
    int32_t timebaseNum{1};
    int32_t timebaseDen{1};
    std::shared_ptr<uint8_t> data;      // RGBA 数据

    // unsigned int textureId{0};          // OpenGL 纹理 ID

    std::weak_ptr<std::function<void()>> releaseCallback;

    float GetTimeStamp() const {
        return pts * 1.0f * timebaseNum * timebaseDen;
    }
    virtual ~IVideoFrame() {
        if (auto lockedPtr = releaseCallback.lock()) {
            (*lockedPtr)();
        }
    }
};

}

