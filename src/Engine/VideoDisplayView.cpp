#include "VideoDisplayView.h"

namespace av {

static const char* vertexShaderSource = R"(
    #version 330 core
    layout(location = 0) in vec3 aPos;
    layout(location = 1) in vec2 aTexCoord;

    out vec2 TexCoord;

    void main()
    {
        gl_Position = vec4(aPos, 1.0);
        TexCoord = aTexCoord;
    }
)";

static const char* fragmentShaderSource = R"(
    #version 330 core
    out vec4 FragColor;

    in vec2 TexCoord;

    uniform sampler2D texture1;

    void main()
    {
        FragColor = texture(texture1, TexCoord);
    }
)";


IVideoDisplayView* IVideoDisplayView::Create() { 
    return new VideoDisplayView(); 
}

VideoDisplayView::~VideoDisplayView() { 
    Clear(); 
}

void VideoDisplayView::Clear() {
    SyncNotifier notifier;
    if (m_taskPool) {
        m_taskPool->SubmitTask([&]() {
            if (m_shaderProgram > 0) glDeleteProgram(m_shaderProgram);
            std::lock_guard<std::mutex> lock(m_videoFrameMutex);
            m_videoFrame = nullptr;
            notifier.Notify();
        });
    }
    notifier.Wait();
}

void VideoDisplayView::SetTaskPool(std::shared_ptr<TaskPool> taskPool) {
    m_taskPool = taskPool; 
}

void VideoDisplayView::InitializeGL() {
    m_shaderProgram = GLUtils::CompileAndLinkProgram(vertexShaderSource, fragmentShaderSource);

    // 坐标和纹理
    float vertices[] = {
        1.0f,  1.0f,  0.0f, 1.0f, 1.0f,
        1.0f,-1.0f, 0.0f, 1.0f, 0.0f,
        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
        -1.0f, 1.0f,  0.0f, 0.0f, 1.0f
    };

    unsigned int indices[] = {0, 1, 3, 1, 2, 3};

    glGenVertexArrays(1, &m_VAO);
    glGenBuffers(1, &m_VBO);
    glGenBuffers(1, &m_EBO);

    glBindVertexArray(m_VAO);

    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

void VideoDisplayView::SetDisplaySize(int width, int height) { 
    glViewport(0, 0, width, height);
}

void VideoDisplayView::Render(std::shared_ptr<IVideoFrame> videoFrame, EContentMode mode) {
    if (!videoFrame) {
        return;
    }
    std::lock_guard<std::mutex> lock(m_videoFrameMutex);
    m_videoFrame = videoFrame;
    m_mode = mode;
}

void VideoDisplayView::Render(int width, int height, float red, float green, float blue) {
    // 清屏
    glClearColor(red, green, blue, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    if (!m_videoFrame) {
        return;
    }

    // 未设置纹理时进行设置
    if (!m_videoFrame->textureId) {
        glGenTextures(1, &m_videoFrame->textureId);
        glBindTexture(GL_TEXTURE_2D, m_videoFrame->textureId);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_videoFrame->width, m_videoFrame->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_videoFrame->data.get());
        // 设置环绕模式
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    } else {
        glBindTexture(GL_TEXTURE_2D, m_videoFrame->textureId);
    }

    glUseProgram(m_shaderProgram);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_videoFrame->textureId);

    glBindVertexArray(m_VAO);

    // 根据模式(缩放模式)显示帧
    // ScaleToFill: 强制填满
    // ScaleAspectFit: 保持比例，使用黑边填充
    // ScaleAspectFill: 保持比例，填满屏幕(裁剪)
    switch (m_mode) {
        case EContentMode::kScaleToFill:
            glViewport(0, 0, width, height);
            break;
        case EContentMode::kScaleAspectFit: {
            float aspectRatio = static_cast<float>(m_videoFrame->width) / m_videoFrame->height;
            float screenAspectRatio = static_cast<float>(width) / height;
            if (aspectRatio > screenAspectRatio) {
                int newHeight = static_cast<int>(width / aspectRatio);
                glViewport(0, (height - newHeight) / 2, width, newHeight);
            } else {
                int newWidth = static_cast<int>(height * aspectRatio);
                glViewport((width - newWidth) / 2, 0, newWidth, height);
            }
            break;
        }
        case EContentMode::kScaleAspectFill: {
            float aspectRatio = static_cast<float>(m_videoFrame->width) / m_videoFrame->height;
            float screenAspectRatio = static_cast<float>(width) / height;
            if (aspectRatio > screenAspectRatio) {
                int newWidth = static_cast<int>(height * aspectRatio);
                glViewport((width - newWidth) / 2, 0, newWidth, height);
            } else {
                int newHeight = static_cast<int>(width / aspectRatio);
                glViewport(0, (height - newHeight) / 2, width, newHeight);
            }
            break;
        }
    }

}


}