#include "Interface/IVideoDisplayView.h"

#include "Core/SyncNotifier.h"
#include "Core/TaskPool.h"
#include "Utils/GLUtils.h"
#include <iostream>

namespace av {

class VideoDisplayView : public IVideoDisplayView { 
public:
    VideoDisplayView() = default;
    ~VideoDisplayView();

    void SetTaskPool(std::shared_ptr<TaskPool> taskPool) override;
    // 编译着色器程序 创建 VAO VBO 设置顶点属性（位置+纹理坐标）
    void InitializeGL() override;
    // 设置窗口大小
    void SetDisplaySize(int width, int height) override;
    // 线程安全地更新渲染状态
    void Render(std::shared_ptr<IVideoFrame> videoFrame, EContentMode mode) override;
    // 实际渲染函数，渲染当前帧
    void Render(int width, int height, float red, float green, float blue) override;
    void Clear() override;
private:
    std::shared_ptr<TaskPool> m_taskPool;

    // 当前需要展示的视频帧
    std::shared_ptr<IVideoFrame> m_videoFrame;
    std::mutex m_videoFrameMutex;

    unsigned int m_shaderProgram{0};
    unsigned int m_VAO{0};
    unsigned int m_VBO{0};
    unsigned int m_EBO{0};

    // 显示模式
    EContentMode m_mode;
};


}