//
// Created by Nettal on 2024/5/28.
//

#ifndef CAPTURE_ABSTRACTFRAMEPROCESSOR_H
#define CAPTURE_ABSTRACTFRAMEPROCESSOR_H

#include "CaptureShared.h"
#include "AbstractCapture.h"

class AbstractFrameProcessor {
public:
    virtual void preCapture(AbstractCapture *capture) = 0;

    virtual void endCapture(AbstractCapture *capture) = 0;

    virtual void receiveFrame(OnFrameArriveParameter *para) = 0;
};


#endif //CAPTURE_ABSTRACTFRAMEPROCESSOR_H
