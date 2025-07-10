#pragma once

#include <memory>
#include "Define/FileWriterParameters.h"
#include "Define/IAVPacket.h"
#include "Define/IAudioSamples.h"

namespace av {

struct IAudioEncoder {
    struct Listener {
        virtual void OnAudioEncoderNotifyPacket(std::shared_ptr<IAVPacket>) = 0;
        virtual void OnAudioEncoderNotifyFinished() = 0;
        virtual ~Listener() = default;
    };
    virtual void SetListener(Listener* listener) = 0;
    virtual bool Configure(FileWriterParameters& parameters, int flags) = 0;
    virtual void NotifyAudioSamples(std::shared_ptr<IAudioSamples> audioSamples) = 0;
};

}