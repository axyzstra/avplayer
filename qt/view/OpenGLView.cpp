#include "OpenGLView.h"

namespace av {

OpenGLView::OpenGLView(QWidget *parent) : QOpenGLWidget(parent) {
    m_videoDisplayView = std::shared_ptr<IVideoDisplayView>(IVideoDisplayView::Create());
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    
    // 连接视频显示视图的更新信号
    auto videoDisplayView = dynamic_cast<QObject*>(m_videoDisplayView.get());
    if (videoDisplayView) {
        QObject::connect(videoDisplayView, SIGNAL(updateRequested()),
                         this, SLOT(update()));
    }
}

void OpenGLView::initializeGL() {
    if (m_videoDisplayView) m_videoDisplayView->InitializeGL();
}

void OpenGLView::resizeGL(int w, int h) {
    m_glWidth = w * devicePixelRatio();
    m_glHeight = h * devicePixelRatio();
}

void OpenGLView::paintGL() {
    if (m_videoDisplayView) m_videoDisplayView->Render(m_glWidth, m_glHeight, 0.0, 0.0, 0.0);
}


}