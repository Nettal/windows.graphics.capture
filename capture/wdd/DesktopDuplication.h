//
// Created by Nettal on 2024/6/1.
//

#ifndef CAPTURE_DESKTOPDUPLICATION_H
#define CAPTURE_DESKTOPDUPLICATION_H


#include <memory>
#include <dxgi1_2.h>
#include "capture/AbstractCapture.h"
#include "capture/AbstractFrameProcessor.h"
#include "capture/D3D11Context.h"

class DesktopDuplication : public AbstractCapture {
    std::shared_ptr<D3D11Context> ctx{};
    std::shared_ptr<AbstractFrameProcessor> frameProcessor{};
    IDXGIOutputDuplication *outputDuplication{};
    D3D11_TEXTURE2D_DESC texDesc{};
    bool running = false;

    static IDXGIOutputDuplication *getCurrentDuplication(ID3D11Device *d3d11Device);

    void updateOutputDuplication();

public:
    explicit DesktopDuplication(std::shared_ptr<D3D11Context> ctx,
                                std::shared_ptr<AbstractFrameProcessor> frameProcessor);

    void start() final;

    void stop() final;

    SIZE2D currentFrameSize() final;
};


#endif //CAPTURE_DESKTOPDUPLICATION_H
