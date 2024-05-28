//
// Created by Nettal on 2024/5/28.
//

#ifndef WGC_WGCCAPTURE_H
#define WGC_WGCCAPTURE_H


#include "capture/AbstractCapture.h"
#include "capture/D3D11Context.h"

class WGCCapture : public AbstractCapture {
    void *wgc_c_internal{};
    SIZE2D frameSize{};
    bool running{};
public:
    explicit WGCCapture(const D3D11Context &ctx, HMONITOR monitorToCapture = nullptr, int bufferNum = 2);

    void start(OnFrameArrive para) final;

    void stop() final;

    SIZE2D currentFrameSize() final;
};


#endif //WGC_WGCCAPTURE_H
