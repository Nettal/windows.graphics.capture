//
// Created by Nettal on 2024/5/25.
//

#include <iostream>
#include "WindowsGraphicsCapture.h"


WindowsGraphicsCapture::WindowsGraphicsCapture() {
    D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, D3D11_CREATE_DEVICE_BGRA_SUPPORT,
                      nullptr, 0, D3D11_SDK_VERSION, &d3d11Device, nullptr, nullptr);
    wgc_c_internal = wgc_initial_everything(receiveWGCFrame, d3d11Device, this);
}

void
WindowsGraphicsCapture::receiveWGCFrame(OnFrameArriveParameter *onFrameArriveParameter, OnFrameArriveRet *arriveRet) {
    auto this_ = reinterpret_cast<WindowsGraphicsCapture *>(onFrameArriveParameter->userPtr);
    arriveRet->running = this_->running;
}

void WindowsGraphicsCapture::doCapture() {
    running = 1;

    // blocked
    wgc_do_capture_on_this_thread(wgc_c_internal);
}

void WindowsGraphicsCapture::stopCapture() {
    running = 0;
}
