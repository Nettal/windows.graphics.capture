//
// Created by Nettal on 2024/5/28.
//

#ifndef WGC_WGCCAPTURE_H
#define WGC_WGCCAPTURE_H


#include <memory>
#include "capture/AbstractCapture.h"
#include "capture/D3D11Context.h"
#include "capture/AbstractFrameProcessor.h"

class WGCCapture : public AbstractCapture {
    void *wgc_c_internal{};
    SIZE2D frameSize{};
    bool running{};
    std::shared_ptr<AbstractFrameProcessor> frameProcessor;
public:
    explicit WGCCapture(const std::shared_ptr<D3D11Context> &ctx,
                        std::shared_ptr<AbstractFrameProcessor> frameProcessor,
                        HMONITOR monitorToCapture = nullptr, int bufferNum = 2);

    void start() final;

    void stop() final;

    SIZE2D currentFrameSize() final;
};


#endif //WGC_WGCCAPTURE_H
