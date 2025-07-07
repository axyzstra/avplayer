#pragma once

#include "Interface/IVideoPipeline.h"
#include "VideoFilter/VideoFilter.h"
#include <thread>
#include <condition_variable>
#include <mutex>


namespace av {

class VideoPipeline : public IVideoPipeline {
public:
    explicit VideoPipeline(GLContext& glContext);
    ~VideoPipeline() override;

    // 添加和移除滤镜
    std::shared_ptr<IVideoFilter> AddVideoFilter(VideoFilterType type) override;
    void RemoveVideoFilter(VideoFilterType type) override;

    void SetListener(Listener* listener) override;

    void NotifyVideoFrame(std::shared_ptr<IVideoFrame> videoFrame) override;
    void NotifyVideoFinished() override;
    void Stop() override;

private:
    // 持续地接收、处理视频帧，并应用各种 OpenGL 滤镜效果
    void ThreadLoop();

    // 将 CPU 内存中的视频帧数据转换为 GPU 可用的纹理，并且在必要时对视频帧进行垂直翻转，为后续的滤镜渲染做准备。
    void PrepareVideoFrame(std::shared_ptr<IVideoFrame> videoFrame);
    // 初始化或调整一个用于临时存储渲染结果的 OpenGL 纹理
    void PrepareTempTexture(int width, int height);

    // 将一系列预设的视频滤镜（VideoFilter）按顺序应用到视频帧上
    // 管理 OpenGL 纹理的输入和输出，实现多个视频滤镜的串联应用
    void RenderVideoFilter(std::shared_ptr<IVideoFrame> videoFrame);

private:
    GLContext m_sharedGLContext;

    std::mutex m_queueMutex;
    std::list<std::shared_ptr<IVideoFrame>> m_frameQueue;

    std::shared_ptr<VideoFilter> m_flipVerticalFilter;  // 垂直翻转滤镜

    std::recursive_mutex m_listenerMutex;
    IVideoPipeline::Listener* m_listener{nullptr};

    // 滤镜列表(包括灰度、反色、贴纸)，翻转单列
    std::mutex m_videoFilterMutex;
    std::list<std::shared_ptr<VideoFilter>> m_videoFilters;
    std::list<std::shared_ptr<VideoFilter>> m_removedVideoFilters;


    // 多线程相关
    std::condition_variable m_queueCondVar;
    std::shared_ptr<std::thread> m_thread;
    bool m_abort{false};

    struct TextureInfo {
        unsigned int id{0};
        int width{0};
        int height{0};
    };
    TextureInfo m_tempTexture;

};

}