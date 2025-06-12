#include "Interface/IVideoDecoder.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
}
#include <mutex>
#include <list>
#include <iostream>

namespace av {

class VideoDecoder : public IVideoDecoder {
public:
    VideoDecoder();
    ~VideoDecoder();

    void SetStream(struct AVStream* stream) override;
    void Decode(std::shared_ptr<IAVPacket> packet) override;
    void SetListener(IVideoDecoder::Listener* listener) override;

private:
    void CleanContext();
    void DecodeAVPacket();



private:
    // 监听器
    IVideoDecoder::Listener* m_listener;
    std::recursive_mutex m_listenerMutex;


    // 解码
    AVCodecContext* m_codecContext{nullptr};
    std::mutex m_codecContextMutex;

    // 缩放器
    SwsContext* m_swsContext{nullptr};
    AVRational m_timeBase{AVRational{1, 1}};

    // packet 队列
    std::list<std::shared_ptr<IAVPacket>> m_packetQueue;
    std::mutex m_packetQueueMutex;


    std::atomic<int> m_pipelineResourceCount{3};
    std::shared_ptr<std::function<void()>> m_pipelineReleaseCallback;


};


}