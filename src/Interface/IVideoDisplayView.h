#pragma once

#include <QOpenGLContext>
#include <QOpenGLWidget>
#include <memory>

#include "Define/IVideoFrame.h"
#include "IGLContext.h"
#include "Core/TaskPool.h"

namespace av {

struct IVideoDisplayView {
    enum class EContentMode {
        kScaleToFill = 0,
        kScaleAspectFit,
        kScaleAspectFill
    };

    virtual void SetDisplaySize(int width, int height) = 0;
    virtual void SetTaskPool(std::shared_ptr<TaskPool> taskPool) = 0;
    virtual void InitializeGL() = 0;
    virtual void Render(std::shared_ptr<IVideoFrame> videoFrame, EContentMode mode) = 0;
    virtual void Render(int width, int height, float red, float green, float blue) = 0;
    virtual void Clear() = 0;

    virtual ~IVideoDisplayView() = default;

    static IVideoDisplayView *Create();
};

}