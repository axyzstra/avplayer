#pragma once

namespace av {

struct IPlaybackListener {
    virtual void NotifyPlaybackStarted() = 0;
    virtual void NotifyPlaybackTimeChanged(float timeStamp, float duration) = 0;
    virtual void NotifyPlaybackPaused() = 0;
    virtual void NotifyPlaybackEOF() = 0;

    virtual ~IPlaybackListener() = default;
};

}