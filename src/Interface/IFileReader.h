#pragma once

#include "Define/IAudioSamples.h"
#include "Define/IVideoFrame.h"


namespace av {

struct IFileReader {
    struct Listener {
        virtual void OnFileReaderNotifyAudioSamples(std::shared_ptr<IAudioSamples>) = 0;
        virtual void OnFileReaderNotifyVideoFrame(std::shared_ptr<IVideoFrame>) = 0;
        virtual ~Listener() = default;
    };
    virtual void SetListener(Listener* listener) = 0;
    virtual bool Open(const std::string& filePath) = 0;
    virtual ~IFileReader() = default;
    static IFileReader* Create();
};

}