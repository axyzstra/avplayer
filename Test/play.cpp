#include "Engine/Player.h"
#include "IGLContext.h"
#include "Engine/VideoDisplayView.h"
#include "VideoFilter/GrayFilter.h"
#include <QApplication>
#include <QOpenGLContext>
#include <QOffscreenSurface>
#include <QOpenGLFunctions>
#include <QThread>
#include <QWindow>
#include <QTimer>
#include <QOpenGLWidget>
#include <iostream>
#include <memory>
#include <thread>
#include <chrono>
#include <QDebug>
using namespace av;
using namespace std;

class TestPlaybackListener : public IPlaybackListener {
public:
    void NotifyPlaybackStarted() override {
        std::cout << "Playback started" << std::endl;
    }
    
    void NotifyPlaybackPaused() override {
        std::cout << "Playback paused" << std::endl;
    }
    
    void NotifyPlaybackTimeChanged(float currentTime, float duration) override {
        std::cout << "Time: " << currentTime << " / " << duration << " seconds" << std::endl;
    }
    
    void NotifyPlaybackEOF() override {
        std::cout << "Playback finished" << std::endl;
    }
};


int main(int argc, char *argv[]) {
    std::cout << "Start Play..." << std::endl;
    
    QApplication app(argc, argv);
    
    // 设置默认OpenGL格式
    QSurfaceFormat format;
    format.setVersion(3, 3);
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setDepthBufferSize(24);
    format.setStencilBufferSize(8);
    format.setSwapBehavior(QSurfaceFormat::DoubleBuffer);
    QSurfaceFormat::setDefaultFormat(format);
    
    // 创建OpenGL上下文
    QOpenGLContext* mainContext = new QOpenGLContext();
    mainContext->setFormat(format);
    
    if (!mainContext->create()) {
        std::cerr << "OpenGL create fail...\n";
        delete mainContext;
        return -1;
    }
    
    // 创建离屏表面
    QOffscreenSurface* surface = new QOffscreenSurface();
    surface->setFormat(mainContext->format());
    surface->create();
    
    if (!surface->isValid()) {
        std::cerr << "Failed to create valid surface\n";
        delete surface;
        delete mainContext;
        return -1;
    }
    
    // 激活上下文
    if (!mainContext->makeCurrent(surface)) {
        std::cerr << "Failed to make context current\n";
        delete surface;
        delete mainContext;
        return -1;
    }
    
    std::cout << "OpenGL context Create success" << std::endl;
    
    // 初始化你的OpenGL组件
    std::unique_ptr<GLContext> glContext;
    std::unique_ptr<Player> player;
    
    try {
        // 确保在正确的上下文中初始化
        mainContext->makeCurrent(surface);
        
        glContext = std::make_unique<GLContext>(mainContext);
        glContext->Initialize();
        
        player = std::make_unique<Player>(*glContext);
        
        auto listener = std::make_shared<TestPlaybackListener>();
        auto displayView = std::make_shared<VideoDisplayView>();
        
        player->SetPlaybackListener(listener);
        player->AttachDisplayView(displayView);
        
        std::string filePath = "F:/test/1.mp4";
        
        // 执行播放操作
        QTimer::singleShot(100, [&]() {
            mainContext->makeCurrent(surface);
            
            player->Open(filePath);
            player->Play();
            std::cout << "Start Play..." << std::endl;
            
            // 10秒后暂停
            QTimer::singleShot(10000, [&]() {
                mainContext->makeCurrent(surface);
                player->Pause();
                std::cout << "Paused" << std::endl;
                
                // 2秒后跳转并播放
                QTimer::singleShot(2000, [&]() {
                    mainContext->makeCurrent(surface);
                    player->SeekTo(0.5f);
                    player->Play();
                    std::cout << "skip and play" << std::endl;
                    
                    // 5秒后暂停并添加滤镜
                    QTimer::singleShot(5000, [&]() {
                        mainContext->makeCurrent(surface);
                        player->Pause();
                        auto filter = player->AddVideoFilter(VideoFilterType::kGray);
                        std::cout << "Gray Filter" << std::endl;
                        
                        // 清理并退出
                        QTimer::singleShot(1000, [&]() {
                            // 清理资源
                            mainContext->makeCurrent(surface);
                            
                            player->DetachDisplayView(displayView);
                            player.reset();
                            
                            mainContext->doneCurrent();
                            glContext->Destroy();
                            glContext.reset();
                            
                            delete surface;
                            delete mainContext;
                            
                            std::cout << "cleaned" << std::endl;
                            QApplication::quit();
                        });
                    });
                });
            });
        });
        
    } catch (const std::exception& e) {
        std::cerr << "Ini fail: " << e.what() << std::endl;
        
        // 清理资源
        if (player) {
            player.reset();
        }
        if (glContext) {
            glContext.reset();
        }
        
        mainContext->doneCurrent();
        delete surface;
        delete mainContext;
        
        return -1;
    }
    
    return app.exec();
}