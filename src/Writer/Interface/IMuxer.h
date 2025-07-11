#pragma once

#include <string>

#include "Define/FileWriterParameters.h"
#include "Define/IAVPacket.h"

namespace av {

struct IMuxer {
    virtual bool Configure(const std::string& outputFilePath, FileWriterParameters& parameters, int flags) = 0;
    virtual void NotifyAudioPacket(std::shared_ptr<IAVPacket> packet) = 0;
    virtual void NotifyVideoPacket(std::shared_ptr<IAVPacket> packet) = 0;
    virtual void NotifyAudioFinished() = 0;
    virtual void NotifyVideoFinished() = 0;

    virtual ~IMuxer() = default;

    static IMuxer* Create();
};

}  // namespace av