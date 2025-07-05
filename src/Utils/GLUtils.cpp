#include "GLUtils.h"
#include <QOpenGLContext>
#include <QImage>
#include <QDebug>

namespace av {

// 编译着色器的辅助函数
unsigned int GLUtils::CompileShader(unsigned int type, const char* source) {
    // 获取当前 OpenGL 上下文的函数指针
    QOpenGLFunctions* gl = QOpenGLContext::currentContext()->functions();
    
    unsigned int shader = gl->glCreateShader(type);
    gl->glShaderSource(shader, 1, &source, nullptr);
    gl->glCompileShader(shader);

    int success;
    char infoLog[512];
    gl->glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        gl->glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "ERROR::SHADER::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    return shader;
}

unsigned int GLUtils::CompileAndLinkProgram(const char* vertexShaderSource, const char* fragmentShaderSource) {
    QOpenGLFunctions* gl = QOpenGLContext::currentContext()->functions();
    
    auto vertexShader = CompileShader(GL_VERTEX_SHADER, vertexShaderSource);
    auto fragmentShader = CompileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
    auto shaderProgram = gl->glCreateProgram();
    gl->glAttachShader(shaderProgram, vertexShader);
    gl->glAttachShader(shaderProgram, fragmentShader);
    gl->glLinkProgram(shaderProgram);
    
    // 检查链接状态
    int success;
    char infoLog[512];
    gl->glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        gl->glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
        std::cerr << "ERROR::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }
    
    gl->glDeleteShader(vertexShader);
    gl->glDeleteShader(fragmentShader);
    return shaderProgram;
}

bool GLUtils::CheckGLError(const char* tag) {
    QOpenGLFunctions* gl = QOpenGLContext::currentContext()->functions();
    GLenum error = gl->glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "OpenGL Error";
        if (tag) {
            std::cerr << " [" << tag << "]";
        }
        std::cerr << ": 0x" << std::hex << error << std::endl;
        return false;
    }
    return true;
}

GLuint GLUtils::GenerateTexture(int width, int height, GLenum internalFormat, GLenum format) {
    QOpenGLFunctions* gl = QOpenGLContext::currentContext()->functions();
    
    GLuint texture;
    gl->glGenTextures(1, &texture);
    gl->glBindTexture(GL_TEXTURE_2D, texture);
    gl->glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, nullptr);
    
    // 设置纹理参数
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    gl->glBindTexture(GL_TEXTURE_2D, 0);
    return texture;
}

GLuint GLUtils::LoadImageFileToTexture(const std::string& imagePath, int& width, int& height) {
    QOpenGLFunctions* gl = QOpenGLContext::currentContext()->functions();
    
    GLuint texture = 0;
    
    // 使用 Qt 加载图片
    QImage image(QString::fromStdString(imagePath));
    if (image.isNull()) {
        std::cerr << "Failed to load texture: " << imagePath << std::endl;
        return 0;
    }
    
    // 转换为 OpenGL 格式
    QImage glImage = image.convertToFormat(QImage::Format_RGBA8888);
    width = glImage.width();
    height = glImage.height();
    
    gl->glGenTextures(1, &texture);
    gl->glBindTexture(GL_TEXTURE_2D, texture);
    gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, glImage.bits());
    gl->glGenerateMipmap(GL_TEXTURE_2D);
    
    // 设置纹理参数
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    gl->glBindTexture(GL_TEXTURE_2D, 0);
    return texture;
}

}