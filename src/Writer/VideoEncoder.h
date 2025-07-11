#pragma once

#include "IGLContext.h"
#include "Interface/IVideoEncoder.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}
#include <mutex>
#include <thread>

namespace av {

class VideoFilter;

class VideoEncoder : public IVideoEncoder {
public:
    explicit VideoEncoder(GLContext& glContext);
    ~VideoEncoder();
    void SetListener(Listener* listener) override;
    bool Configure(FileWriterParameters& parameters, int flags) override;
    void NotifyVideoFrame(std::shared_ptr<IVideoFrame> videoFrame) override;

private:
    void ThreadLoop();
    void StopThread();
    void FlipVideoFrame(std::shared_ptr<IVideoFrame> videoFrame);
    void ConvertTextureToFrame(unsigned int textureId, int width, int height);
    void PrepareAndEncodeVideoFrame(std::shared_ptr<IVideoFrame> videoFrame);
    void EncodeVideoFrame(const AVFrame* frame);

private:
    GLContext m_sharedGLContext;

    Listener* m_listener{nullptr};
    std::mutex m_listenerMutex;

    // AVPacket 队列
    std::list<std::shared_ptr<IVideoFrame>> m_videoFrameQueue;
    std::condition_variable m_cond;
    std::mutex m_mutex;

    std::thread m_thread;
    std::atomic<bool> m_abort{false};

    std::shared_ptr<VideoFilter> m_flipVerticalFilter;  // 垂直翻转滤镜
    unsigned int m_textureId{0};

    AVCodecContext* m_encodeCtx{nullptr};
    SwsContext* m_swsCtx{nullptr};
    AVFrame* m_avFrame{nullptr};
    AVPacket* m_avPacket{nullptr};
    std::vector<uint8_t> m_rgbaData;
};

}  // namespace av