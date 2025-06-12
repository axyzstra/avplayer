#pragma once


#include <cstdint>
#include <vector>
#include <functional>
#include <memory>


namespace av {

// 封装和管理 PCM 数据及其相关元数据
struct IAudioSamples {
    int flags{0};               // 标志位
    unsigned int channels;      // 通道数
    unsigned int sampleRate;    // 采样率

    int64_t pts;                // 时间戳
    int64_t duration;           // 持续时间
    int32_t timebaseNum;        // 时间基数分子
    int32_t timebaseDen;        // 时间基数分母

    std::vector<int16_t> pcmData;
    size_t offset{0};

    std::weak_ptr<std::function<void()>> releaseCallback;   // 释放资源的回调函数

    // 获取时间戳
    float GetTimeStamp() const {
        return pts * 1.0f * (timebaseNum / timebaseDen);
    }

    virtual ~IAudioSamples() {
        if (auto lockedptr = releaseCallback.lock()) {
            (*lockedptr)();
        }
    }
};
}