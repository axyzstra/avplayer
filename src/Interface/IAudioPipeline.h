#pragma once

#include "Define/IAudioSamples.h"
#include "IGLContext.h"

namespace av {

struct IAudioPipeline {
    struct Listener {
        virtual void OnAudioPipelineNotifyAudioSamples(std::shared_ptr<IAudioSamples>) = 0;
        virtual void OnAudioPipelineNotifyFinished() = 0;
        virtual ~Listener() = default;
    };

    virtual void SetListener(Listener* listener) = 0;
    virtual void NotifyAudioSamples(std::shared_ptr<IAudioSamples> audioSamples) = 0;
    virtual void NotifyAudioFinished() = 0;

    virtual ~IAudioPipeline() = default;

    static IAudioPipeline* Create(unsigned int channels, unsigned int sampleRate);
};


}