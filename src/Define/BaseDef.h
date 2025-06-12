#pragma once

enum AVFrameFlag {
    kKeyFrame = 1 << 0,     // 关键帧
    kFlush = 1 << 1,        // 刷新
    kEOS = 1 << 2,          // 结束
};
