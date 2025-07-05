#pragma once

#include "Define/IVideoFrame.h"
#include "IGLContext.h"
#include "IVideoFilter.h"
#include <memory>

namespace av {

struct IVideoPipeline {
    struct Listener {
        virtual void OnVideoPipelineNotifyVideoFrame(std::shared_ptr<IVideoFrame>) = 0;
        virtual void OnVideoPipelineNotifyFinished() = 0;
        virtual ~Listener() = default;
    };

    virtual std::shared_ptr<IVideoFilter> AddVideoFilter(VideoFilterType type) = 0;
    virtual void RemoveVideoFilter(VideoFilterType type) = 0;

    virtual void SetListener(Listener* listener) = 0;

    // 多线程相关
    virtual void NotifyVideoFrame(std::shared_ptr<IVideoFrame> videoFrame) = 0;
    virtual void NotifyVideoFinished() = 0;
    virtual void Stop() = 0;

    static IVideoPipeline* Create(GLContext& glContext);

    virtual ~IVideoPipeline() = default;
};


}