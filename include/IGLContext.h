#pragma once


#include <QOffscreenSurface>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <iostream>

namespace av {

class GLContext {
public:
    explicit GLContext(QOpenGLContext *context);
    virtual ~GLContext();

    bool Initialize();      // 初始化 OpenGL 上下文
    void Destroy();         // 销毁 OpenGL 上下文
    void MakeCurrent();     // 激活当前上下文，使其成为当前线程的活动上下文
    void DoneCurrent();     // 释放 OpenGL 上下文
private:
    QOpenGLContext *m_sharedGLContext{nullptr};
    QOpenGLContext *m_context{nullptr};
    QOffscreenSurface *m_surface{nullptr};
};

}