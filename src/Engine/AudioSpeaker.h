#include "Interface/IAudioSpeaker.h"
#include <QAudioDevice>
#include <QAudioFormat>
#include <QAudioSink>
#include <QIODevice>
#include <QMediaDevices>
#include <mutex>

#include <stdint.h>
#include <stdio.h>
#include <iostream>

namespace av {

class AudioSpeaker : public IAudioSpeaker, public QIODevice {
public:
    AudioSpeaker(unsigned int channels, unsigned int sampleRate);
    ~AudioSpeaker() override;

    // 播放音频，将音频加入到队列中
    void PlayAudioSamples(std::shared_ptr<IAudioSamples> audioSamples) override;
    void Stop() override;

private:
    // 继承 QIODevice 需重写方法
    qint64 readData(char *data, qint64 maxlen) override;
    qint64 writeData(const char *data, qint64 len) override { return 0; }
private:
    // QAudioSink *m_audioSink{nullptr};
    QMediaDevices *m_outputDevices{nullptr};        // 管理输出设备
    QAudioDevice outputDevice;                      // 实际输出设备
    QAudioSink *m_audioSinkOutput{nullptr};         // 

    IAudioSamples m_audioSamples;                   // 当前播放的音频样本
    std::list<std::shared_ptr<IAudioSamples>> m_audioSampleList;        // 待播放音频样本队列
    std::mutex m_audioSamplesListMutex;
};


}