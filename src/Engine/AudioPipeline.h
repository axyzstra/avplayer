#pragma once
#include "Interface/IAudioPipeline.h"
#include <mutex>

namespace av {
// 处理中间层，可以在该层进行音频的一些处理，然后交给 AudioSpeaker
class AudioPipeline : public IAudioPipeline {
public:
    AudioPipeline(unsigned int channels, unsigned int sampleRate);
    ~AudioPipeline() override = default;

    void SetListener(Listener* listener) override;
    void NotifyAudioSamples(std::shared_ptr<IAudioSamples> audioSamples) override;
    void NotifyAudioFinished() override;

private:
    unsigned int m_channels{2};
    unsigned int m_sampleRate{44100};

    IAudioPipeline::Listener* m_listener{nullptr};
    std::recursive_mutex m_listenerMutex;
};


}