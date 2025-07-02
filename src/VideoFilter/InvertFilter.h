#pragma once

#include "VideoFilter.h"

namespace av {
// 反色滤镜
class InvertFilter : public VideoFilter {
public:
    InvertFilter();
};
}  // namespace av