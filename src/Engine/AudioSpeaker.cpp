#include "AudioSpeaker.h"


namespace av {

IAudioSpeaker *IAudioSpeaker::Create(unsigned int channels, unsigned int sampleRate) {
    return new AudioSpeaker(channels, sampleRate);
}

AudioSpeaker::AudioSpeaker(unsigned int channels, unsigned int sampleRate) {
    // 获取默认输出设备
    m_outputDevices = new QMediaDevices(nullptr);
    outputDevice = m_outputDevices->defaultAudioOutput();

    // 配置格式
    QAudioFormat format = outputDevice.preferredFormat();
    format.setChannelCount(channels);
    format.setSampleRate(sampleRate);
    format.setSampleFormat(QAudioFormat::Int16);        // 16 位 PCM

    m_audioSinkOutput = new QAudioSink(outputDevice, format);
    m_audioSinkOutput->setBufferSize(16 * 1024);
    open(QIODevice::ReadOnly);
    m_audioSinkOutput->start(this);
}

AudioSpeaker::~AudioSpeaker() {
    close();
    m_audioSinkOutput->stop();
}

void AudioSpeaker::PlayAudioSamples(std::shared_ptr<IAudioSamples> audioSamples) {
    if (!audioSamples) {
        return;
    }
    std::lock_guard<std::mutex> lock(m_audioSamplesListMutex);
    m_audioSampleList.push_back(audioSamples);
}

void AudioSpeaker::Stop() {
    std::lock_guard<std::mutex> lock(m_audioSamplesListMutex);
    m_audioSampleList.clear();
}

qint64 AudioSpeaker::readData(char *data, qint64 maxlen) {
    // 当前音频数据为空或者播放完毕，则取出下一个音频数据进行播放
    if (m_audioSamples.pcmData.empty() || m_audioSamples.offset >= m_audioSamples.pcmData.size()) {
        std::lock_guard<std::mutex> lock(m_audioSamplesListMutex);
        if (m_audioSampleList.empty()) {
            return 0;
        }
        m_audioSamples = *m_audioSampleList.front();
        m_audioSampleList.pop_front();
    }

    size_t bytesToRead = std::min(static_cast<size_t>(maxlen), (m_audioSamples.pcmData.size() - m_audioSamples.offset) * sizeof(int16_t));
    // 将播放音频拷贝到 data
    std::memcpy(data, m_audioSamples.pcmData.data() + m_audioSamples.offset, bytesToRead);
    m_audioSamples.offset += bytesToRead / sizeof(int16_t);
    return bytesToRead;
}


}