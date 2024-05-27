//
// Created by Nettal on 2024/5/25.
//

#include "main.h"
#include "wgc/WindowsGraphicsCapture.h"

int main() {
    auto ctx = D3D11Context();
    WGC_SIZE2D textureSize;
    auto wgc_c_internal = wgc_initial_everything(nullptr, &textureSize, ctx.d3d11Device);
    auto wgc = WindowsGraphicsCapture(ctx, [textureSize, ctx]() {
        return FrameSender{{DXGIMapping{ctx.d3d11Device, textureSize, ctx.deviceCtx},
                            DXGIMapping{ctx.d3d11Device, textureSize, ctx.deviceCtx}}};
    }, textureSize);
    wgc.doCapture();// todo make a wrapper for wgc, wgc should be called in do capture
    wgc_do_capture_on_this_thread(wgc_c_internal, [](OnFrameArriveParameter *para, OnFrameArriveRet *ret) {
        reinterpret_cast<WindowsGraphicsCapture *>(para->userPtr)->receiveWGCFrame(
                para, ret);
    }, &wgc);
}