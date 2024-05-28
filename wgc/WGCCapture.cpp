//
// Created by Nettal on 2024/5/28.
//

#include "WGCCapture.h"
#include "windows_graphics_capture.h"

void WGCCapture::start(AbstractCapture::OnFrameArrive onFrameArrive) {
    running = true;
    struct PARA_ {
        WGCCapture *this_;
        OnFrameArrive onFrameArrive;
    };
    PARA_ para{this, onFrameArrive};
    wgc_do_capture_on_this_thread(wgc_c_internal,
                                  [](OnFrameArriveParameter *para, OnFrameArriveRet *ret) {
                                      auto pPara = reinterpret_cast<PARA_ *>(para->userPtr);
                                      ret->running = pPara->this_->running;
                                      pPara->this_->frameSize = para->frameSize;
                                      pPara->onFrameArrive(para);
                                  }, &para);
}

void WGCCapture::stop() {
    running = false;
}

SIZE2D WGCCapture::currentFrameSize() {
    return frameSize;
}

WGCCapture::WGCCapture(const D3D11Context &ctx) {
    wgc_c_internal = wgc_initial_everything(nullptr, &frameSize, ctx.d3d11Device);
}
