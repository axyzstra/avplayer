#pragma once

#include "DeMuxer.h"
#include "AudioDecoder.h"
#include "VideoDecoder.h"
#include "Interface/IFileReader.h"


namespace av {

class FileReader : public IFileReader,
                   public IDeMuxer::Listener,
                   public IAudioDecoder::Listener,
                   public IVideoDecoder::Listener {
public:
    FileReader();
    ~FileReader() override;

    void SetListener(IFileReader::Listener* listener) override;
    bool Open(const std::string& filePath) override;


private:
    // IDeMuxer::Listener
    // 当 packet 准备好后通知解码器功能讲 packet 添加到解码队列
    void OnNotifyAudioPacket(std::shared_ptr<IAVPacket> packet) override;
    void OnNotifyVideoPacket(std::shared_ptr<IAVPacket> packet) override;


    // IAudioDecoder::Listener
    void OnNotifyAudioSamples(std::shared_ptr<IAudioSamples>) override;

    // IVideoDecoder::Listener
    void OnNotifyVideoFrame(std::shared_ptr<IVideoFrame>) override;


private:
    IFileReader::Listener* m_listener{nullptr};
    std::recursive_mutex m_listenerMutex;

    // 解复用器
    std::shared_ptr<IDeMuxer> m_deMuxer;

    // 解码器
    std::shared_ptr<IAudioDecoder> m_audioDecoder;
    std::shared_ptr<IVideoDecoder> m_videoDecoder;
};


}