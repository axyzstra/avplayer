#pragma once

#include <QDockWidget>
#include <QLabel>
#include <QMainWindow>
#include <QOpenGLContext>
#include <QOffscreenSurface>
#include <QSlider>
#include <QSplitter>
#include <QVBoxLayout>
#include <memory>

#include "Engine/Player.h"
#include "IGLContext.h"
#include "UI/PlayerWidget.h"

class MainWindow;

class PlaybackListener : public av::IPlaybackListener {
public:
    explicit PlaybackListener(MainWindow *window);
    void NotifyPlaybackStarted() override;
    void NotifyPlaybackTimeChanged(float timeStamp, float duration) override;
    void NotifyPlaybackPaused() override;
    void NotifyPlaybackEOF() override;

private:
    MainWindow *m_window{nullptr};
};

class MainWindow : public QWidget {
    Q_OBJECT
public:
    MainWindow();
    ~MainWindow() override;
    
    // 添加加载视频的公共方法
    void loadVideo(const QString& filePath);

protected:
    void resizeEvent(QResizeEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;

private slots:
    void onSliderMoved(int value);

private:
    friend class PlaybackListener;
    
    // UI组件
    PlayerWidget *m_playerWidget{nullptr};
    QSlider *m_progressSlider{nullptr};
    
    // 播放器和回调
    std::shared_ptr<av::IPlayer> m_player;
    std::unique_ptr<PlaybackListener> m_playbackListener;
    
    // OpenGL上下文管理
    QOpenGLContext *m_mainGLContext{nullptr};
    QOffscreenSurface *m_offscreenSurface{nullptr};
};