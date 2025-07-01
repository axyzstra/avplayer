#pragma once

#include "IGLContext.h"

namespace av {

struct IVideoPipeline {
    struct Listener {

    };

    virtual void SetListener(Listener* listener) = 0;
    virtual void Stop() = 0;

    virtual ~IVideoPipeline() = default;
};


}