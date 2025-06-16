#pragma once
#include "Define/IAVPacket.h"
#include "Define/IVideoFrame.h"

struct AVStream;
namespace av {
struct IVideoDecoder {
    
    struct Listener {
        virtual void OnNotifyVideoFrame(std::shared_ptr<IVideoFrame>) = 0;
        virtual ~Listener() = default;
    };


    virtual ~IVideoDecoder() = default;
    static IVideoDecoder* Create();

    virtual void SetStream(struct AVStream* stream) = 0;
    virtual void SetListener(Listener* listener) = 0;
    virtual void Decode(std::shared_ptr<IAVPacket> packet) = 0;

    virtual void Start() = 0;
    virtual void Pause() = 0;
    virtual void Stop() = 0;

    virtual int GetVideoWidth() = 0;
    virtual int GetVideoHeight() = 0;
};

}