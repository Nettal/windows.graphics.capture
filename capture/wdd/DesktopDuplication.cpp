//
// Created by Nettal on 2024/6/1.
//

#include <dxgi1_2.h>

#include <utility>
#include <iostream>
#include "DesktopDuplication.h"
#include <chrono>

#define CHECK_RESULT(x) do{if(FAILED(x)) {fprintf(stderr,"error at %s:%d\n",__FILE__, __LINE__);}} while(0)

SIZE2D DesktopDuplication::currentFrameSize() {
    return {static_cast<int32_t>(texDesc.Width),
            static_cast<int32_t>(texDesc.Height)};
}

void DesktopDuplication::stop() {
    running = false;
}

void DesktopDuplication::start() {
    running = true;
    bool first = true;
    std::vector<RECT> dirties{64};
    uint32_t usedNum = 0;
    while (running) {
        DXGI_OUTDUPL_FRAME_INFO frameInfo{};
        IDXGIResource *dxgiRes{};
        auto hr = outputDuplication->AcquireNextFrame(500, &frameInfo, &dxgiRes);
        if (hr == DXGI_ERROR_WAIT_TIMEOUT)
            continue;
        CHECK_RESULT(hr);
        ID3D11Texture2D *frame{};
        hr = dxgiRes->QueryInterface(__uuidof(ID3D11Texture2D), (void **) (&frame));
        CHECK_RESULT(hr);
        frame->GetDesc(&texDesc);
        if (first) {
            frameProcessor->preCapture(this);
            first = false;
            usedNum++;
            dirties[0] = {0, 0, static_cast<LONG>(texDesc.Width), static_cast<LONG>(texDesc.Height)};
        }
        uint32_t freeByteSize = (dirties.size() - usedNum) * sizeof(RECT);
        hr = outputDuplication->GetFrameDirtyRects(freeByteSize,
                                                   &dirties[usedNum], &freeByteSize);
        if (hr == DXGI_ERROR_MORE_DATA) {
            dirties.resize(usedNum + freeByteSize / sizeof(RECT));
            hr = outputDuplication->GetFrameDirtyRects(freeByteSize,
                                                       &dirties[usedNum], &freeByteSize);
            CHECK_RESULT(hr);
            assert(dirties.size() * sizeof(RECT) == (usedNum * sizeof(RECT) + freeByteSize));
        }
        usedNum += freeByteSize / sizeof(RECT);
        CHECK_RESULT(hr);
        if (frameInfo.AccumulatedFrames <= 1) {
            OnFrameArriveParameter parameter{frame,
                                             (uint64_t) std::chrono::duration_cast<std::chrono::microseconds>(
                                                     std::chrono::system_clock::now().time_since_epoch()).count() * 10,
                                             {static_cast<int32_t>(texDesc.Width),
                                              static_cast<int32_t>(texDesc.Height)}, dirties.data(),
                                             usedNum};
            frameProcessor->receiveFrame(&parameter);
            dirties.clear();
            usedNum = 0;
        }
        frame->Release();
        dxgiRes->Release();
        hr = outputDuplication->ReleaseFrame();
        CHECK_RESULT(hr);
    }
}

DesktopDuplication::DesktopDuplication(std::shared_ptr<D3D11Context> ctx,
                                       std::shared_ptr<AbstractFrameProcessor> frameProcessor) :
        ctx(std::move(ctx)),
        frameProcessor(std::move(frameProcessor)) {
    updateOutputDuplication();
}

IDXGIOutputDuplication *DesktopDuplication::getCurrentDuplication(ID3D11Device *d3d11Device) {
    // Get DXGI device
    IDXGIDevice *dxgiDevice{};
    HRESULT hr = d3d11Device->QueryInterface(__uuidof(IDXGIDevice), (void **) (&dxgiDevice));
    CHECK_RESULT(hr);
    // Get DXGI adapter
    IDXGIAdapter *dxgiAdapter{};
    hr = dxgiDevice->GetParent(__uuidof(IDXGIAdapter), (void **) (&dxgiAdapter));
    CHECK_RESULT(hr);
    // Get output
    IDXGIOutput *dxgiOutput{};
    for (int i = 0; true; i++) {
        hr = dxgiAdapter->EnumOutputs(i, &dxgiOutput);
        CHECK_RESULT(hr);
        if (SUCCEEDED(hr))
            break;
    }
    // QI for Output 1
    IDXGIOutput1 *dxgiOutput1{};
    hr = dxgiOutput->QueryInterface(__uuidof(IDXGIOutput1), (void **) (&dxgiOutput1));
    CHECK_RESULT(hr);
    // Create desktop duplication
    IDXGIOutputDuplication *duplication{};
    hr = dxgiOutput1->DuplicateOutput(d3d11Device, &duplication);
    CHECK_RESULT(hr);
    dxgiDevice->Release();
    dxgiAdapter->Release();
    dxgiOutput->Release();
    dxgiOutput1->Release();
    return duplication;
}

void DesktopDuplication::updateOutputDuplication() {
    if (outputDuplication != nullptr)
        outputDuplication->Release();
    outputDuplication = getCurrentDuplication(ctx->d3d11Device);
}
