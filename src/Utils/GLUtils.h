#pragma once

#include "IGLContext.h"
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>

namespace av {

struct GLUtils {
    // 编译和链接着色器程序(包括顶点和片段两种着色器)
    static unsigned int CompileAndLinkProgram(const char* vertexShaderSource, const char* fragmentShaderSource);
    // 错误检查
    static bool CheckGLError(const char* tag = "");
    // 生成纹理
    static GLuint GenerateTexture(int width, int height, GLenum internalFormat, GLenum format);
    // 将图片加载为纹理
    static GLuint LoadImageFileToTexture(const std::string& imagePath, int& width, int& height);
private:
    // 编译着色器的辅助函数
    static unsigned int CompileShader(unsigned int type, const char* source);
};

}