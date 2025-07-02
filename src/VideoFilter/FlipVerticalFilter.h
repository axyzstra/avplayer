#pragma once

#include "VideoFilter.h"

namespace av {
// 翻转滤镜
class FlipVerticalFilter : public VideoFilter {
public:
    FlipVerticalFilter();
};
}