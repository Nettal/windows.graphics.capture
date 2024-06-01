//
// Created by Nettal on 2024/5/28.
//

#include "WGCCapture.h"

#include <utility>
#include "windows_graphics_capture.h"

void WGCCapture::start() {
    frameProcessor->preCapture(this);
    running = true;
    wgc_do_capture_on_this_thread(wgc_c_internal,
                                  [](WGCOnFrameArriveParameter *para, WGCOnFrameArriveRet *ret) {
                                      auto this_ = reinterpret_cast<WGCCapture *>(para->userPtr);
                                      ret->running = this_->running;
                                      this_->frameSize = para->frameSize;
                                      RECT dirty = {0, 0, para->frameSize.width, para->frameSize.height};
                                      OnFrameArriveParameter parameter{para->d3d11Texture2D, para->systemRelativeTime,
                                                                       para->frameSize.width, para->frameSize.height,
                                                                       &dirty, 1};
                                      this_->frameProcessor->receiveFrame(&parameter);
                                  }, this);
}

void WGCCapture::stop() {
    running = false;
}

SIZE2D WGCCapture::currentFrameSize() {
    return {frameSize.width, frameSize.height};
}

WGCCapture::WGCCapture(const std::shared_ptr<D3D11Context> &ctx, std::shared_ptr<AbstractFrameProcessor> frameProcessor,
                       HMONITOR monitorToCapture, int bufferNum) : frameProcessor(std::move(frameProcessor)) {
    wgc_c_internal = wgc_initial_everything(monitorToCapture, &frameSize, ctx->d3d11Device, bufferNum);
}
