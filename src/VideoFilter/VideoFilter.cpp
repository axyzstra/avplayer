#include "VideoFilter.h"

namespace av {

VideoFilter::~VideoFilter() {
    glDeleteProgram(m_shaderProgram);
    glDeleteVertexArrays(1, &m_vao);
    glDeleteBuffers(1, &m_vbo);
}

void VideoFilter::SetFloat(const std::string& name, float value) { m_floatValues[name] = value; }

float VideoFilter::GetFloat(const std::string& name) { return m_floatValues[name]; }

void VideoFilter::SetInt(const std::string& name, int value) { m_intValues[name] = value; }

int VideoFilter::GetInt(const std::string& name) { return m_intValues[name]; }

void VideoFilter::SetString(const std::string& name, const std::string& value) { m_stringValues[name] = value; }

std::string VideoFilter::GetString(const std::string& name) { return m_stringValues[name]; }

void VideoFilter::Initialize() {
    if (m_initialized) {
        return;
    }
    m_initialized = true;
    // 编译链接片段和顶点着色器
    m_shaderProgram = GLUtils::CompileAndLinkProgram(m_description.vertexShaderSource, m_description.fragmentShaderSource);

    // 顶点和纹理坐标
    float vertices[] = {
        -1.0f, 1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 0.0f,
        1.0f,  -1.0f, 1.0f, 0.0f,
        1.0f,  1.0f,  1.0f, 1.0f
    };

    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);

    glBindVertexArray(m_vao);

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), reinterpret_cast<void*>(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    m_uTextureLocation = glGetUniformLocation(m_shaderProgram, "u_texture");
}

VideoFilter* VideoFilter::Create(VideoFilterType type) {
    switch (type) {
        case VideoFilterType::kFlipVertical:
            return new FlipVerticalFilter();
        case VideoFilterType::kGray:
            return new GrayFilter();
        case VideoFilterType::kInvert:
            return new InvertFilter();
        case VideoFilterType::kSticker:
            return new StickerFilter();
        default:
            break;
    }

    return nullptr;
}

bool VideoFilter::PreRender(std::shared_ptr<IVideoFrame> frame, unsigned int outputTexture) {
    Initialize();
    if (!m_initialized) return false;

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, outputTexture, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "ERROR::FRAMEBUFFER::Framebuffer is not complete!" << std::endl;
        return false;
    }

    glViewport(0, 0, frame->width, frame->height);
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(m_shaderProgram);
    return true;
}

bool VideoFilter::Render(std::shared_ptr<IVideoFrame> frame, unsigned int outputTexture) {
    if (!PreRender(frame, outputTexture)) return false;
    if (!MainRender(frame, outputTexture)) return false;
    return PostRender();
}

bool VideoFilter::MainRender(std::shared_ptr<IVideoFrame> frame, unsigned int outputTexture) {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, frame->textureId);
    if (m_uTextureLocation == -1) {
        std::cerr << "ERROR: uniform 'u_texture' not found or inactive!" << std::endl;
    } else {
        glUniform1i(m_uTextureLocation, 0);
    }

    glBindVertexArray(m_vao);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glBindVertexArray(0);
    return true;
}

bool VideoFilter::PostRender() {
    glBindTexture(GL_TEXTURE_2D, 0);
    return true;
}

int VideoFilter::GetUniformLocation(const std::string& name) {
    // 使用try_emplace尝试插入
    auto [it, inserted] = m_uniformLocationCache.try_emplace(
        name,                                                   // key: 变量名
        glGetUniformLocation(m_shaderProgram, name.c_str())     // value: 位置
    );
    // inserted为 true 表示是新插入的，false 表示已存在
    // it->second 就是uniform的位置值
    if (it->second == -1) {
        std::cerr << "Warning: uniform '" << name << "' doesn't exist!" << std::endl;
    }
    return it->second;
}

}