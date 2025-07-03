#pragma once

#include "Define/IAudioSamples.h"
#include <memory>

namespace av {

struct IAudioSpeaker {
    virtual void PlayAudioSamples(std::shared_ptr<IAudioSamples> samples) = 0;
    virtual void Stop() = 0;
    virtual ~IAudioSpeaker() = default;

    static IAudioSpeaker* Create(unsigned int channels, unsigned int sampleRate);
};

}