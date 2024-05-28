//
// Created by Nettal on 2024/5/28.
//

#ifndef WGC_ABSTRACTCAPTURE_H
#define WGC_ABSTRACTCAPTURE_H


#include <functional>
#include "CaptureShared.h"

class AbstractCapture {
public:
    using OnFrameArrive = std::function<void(OnFrameArriveParameter *onFrameArriveParameter)>;

    virtual void start(OnFrameArrive onFrameArrive) = 0;

    virtual void stop() = 0;

    virtual SIZE2D currentFrameSize() = 0;
};

#endif //WGC_ABSTRACTCAPTURE_H
