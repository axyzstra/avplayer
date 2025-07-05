#pragma once

#include "Interface/IVideoDisplayView.h"
#include "IPlaybackListener.h"
#include "IVideoFilter.h"
#include <string>
#include <memory>

namespace av {

struct IPlayer {
    // 视频显示视图增删，例如：缩放，拉伸等视图
    virtual void AttachDisplayView(std::shared_ptr<IVideoDisplayView> displayView) = 0;
    virtual void DetachDisplayView(std::shared_ptr<IVideoDisplayView> displayView) = 0;

    virtual void SetPlaybackListener(std::shared_ptr<IPlaybackListener> listener) = 0;

    virtual bool Open(std::string &filePath) = 0;
    virtual void Play() = 0;
    virtual void Pause() = 0;
    virtual void SeekTo(float progress) = 0;
    virtual bool IsPlaying() = 0;

    // 视频滤镜处理
    virtual std::shared_ptr<IVideoFilter> AddVideoFilter(VideoFilterType type) = 0;
    virtual void RemoveVideoFilter(VideoFilterType type) = 0;

    virtual ~IPlayer() = default;

    static IPlayer *Create(GLContext glContext);
};


}