#pragma once

#include "AudioEncoder.h"
#include "Interface/IFileWriter.h"
#include "Muxer.h"
#include "VideoEncoder.h"

namespace av {

class FileWriter : public IFileWriter, public AudioEncoder::Listener, VideoEncoder::Listener {
public:
    explicit FileWriter(GLContext& glContext);
    ~FileWriter() override;

    bool StartWriter(const std::string& outputFilePath, FileWriterParameters& parameters, int flags) override;
    void StopWriter() override;

    void NotifyAudioSamples(std::shared_ptr<IAudioSamples> audioSamples) override;
    void NotifyVideoFrame(std::shared_ptr<IVideoFrame> videoFrame) override;

    void NotifyAudioFinished() override;
    void NotifyVideoFinished() override;

private:
    // 继承自 AudioEncoder::Listener
    void OnAudioEncoderNotifyPacket(std::shared_ptr<IAVPacket>) override;
    void OnAudioEncoderNotifyFinished() override;

    // 继承自 VideoEncoder::Listener
    void OnVideoEncoderNotifyPacket(std::shared_ptr<IAVPacket>) override;
    void OnVideoEncoderNotifyFinished() override;

private:
    std::shared_ptr<AudioEncoder> m_audioEncoder;
    std::shared_ptr<VideoEncoder> m_videoEncoder;
    std::shared_ptr<Muxer> m_muxer;
};

}