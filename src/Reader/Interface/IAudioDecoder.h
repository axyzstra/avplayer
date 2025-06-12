#pragma once

#include "Define/IAVPacket.h"
#include "Define/IAudioSamples.h"

namespace av {
struct IAudioDecoder {

    struct Listener {
        virtual void OnNotifyAudioSamples(std::shared_ptr<IAudioSamples>) = 0;
        virtual ~Listener() = default;
    };


    virtual ~IAudioDecoder() = default;
    static IAudioDecoder* Create(unsigned int channels, unsigned int sampleRate);

    virtual void SetStream(struct AVStream* stream) = 0;
    virtual void SetListener(Listener* listener) = 0;
    virtual void Decode(std::shared_ptr<IAVPacket> packet) = 0;
};
}