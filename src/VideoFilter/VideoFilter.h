#pragma once

#include "Define/IVideoFrame.h"
#include "IVideoFilter.h"
#include "Utils/GLUtils.h"
#include "FlipVerticalFilter.h"
#include "GrayFilter.h"
#include "InvertFilter.h"
#include "StickerFilter.h"
#include <string>
#include <unordered_map>

namespace av {

class VideoFilter : public IVideoFilter {
public:
    VideoFilter() = default;
    ~VideoFilter();

    VideoFilterType GetType() const override { return m_description.type; }

    void SetFloat(const std::string& name, float value) override;
    float GetFloat(const std::string& name) override;

    void SetInt(const std::string& name, int value) override;
    int GetInt(const std::string& name) override;

    void SetString(const std::string& name, const std::string& value) override;
    std::string GetString(const std::string& name) override;


    bool Render(std::shared_ptr<IVideoFrame> frame, unsigned int outputTexture);
    static VideoFilter* Create(VideoFilterType type);

protected:
    // 初始化，包括编译着色器，创建 VAO VBO，设置纹理等信息
    virtual void Initialize();
    // 渲染，绑定纹理，绘制图形
    virtual bool MainRender(std::shared_ptr<IVideoFrame> frame, unsigned int outputTexture);
    // 渲染准备 glUseProgram()
    bool PreRender(std::shared_ptr<IVideoFrame> frame, unsigned int outputTexture);
    // 渲染后清理
    bool PostRender();
    // 缓存 uniform 位置，避免多次使用 GPU 查询
    int GetUniformLocation(const std::string& name);

protected:
    struct VideoFilterDescription {
        VideoFilterType type{VideoFilterType::kNone};
        const char* vertexShaderSource{nullptr};
        const char* fragmentShaderSource{nullptr};
    };

protected:
    // 着色器类型和代码
    VideoFilterDescription m_description;

    bool m_initialized{false};
    // OpenGL 中的常用变量 VAO VBO 和着色器程序
    unsigned int m_shaderProgram{0};
    unsigned int m_vao{0};
    unsigned int m_vbo{0};

    std::unordered_map<std::string, int> m_uniformLocationCache;
    std::unordered_map<std::string, float> m_floatValues;
    std::unordered_map<std::string, int> m_intValues;
    std::unordered_map<std::string, std::string> m_stringValues;

    int m_uTextureLocation;
};

}