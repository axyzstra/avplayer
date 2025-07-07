#pragma once

#include <QOpenGLContext>
#include <QOpenGLWidget>
#include <memory>
#include "Interface/IVideoDisplayView.h"


namespace av {

class OpenGLView : public QOpenGLWidget {
    Q_OBJECT

public:
    explicit OpenGLView(QWidget *parent = nullptr);
    ~OpenGLView() override = default;

    std::shared_ptr<IVideoDisplayView> GetVideoDisplayView() {
        return m_videoDisplayView;
    }

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

private:
    int m_glWidth{0};
    int m_glHeight{0};
    std::shared_ptr<IVideoDisplayView> m_videoDisplayView;

};


}