//
// Created by Nettal on 2024/5/25.
//

#include "main.h"
#include "capture/WindowsGraphicsCapture.h"
#include "capture/wgc/WGCCapture.h"

int main() {
    auto ctx = D3D11Context();
    auto capture = std::make_shared<WGCCapture>(ctx);
    auto sender = std::make_shared<FrameSender>(ctx, capture->currentFrameSize());
    auto wgc = FrameProcessor{ctx, sender, capture};
    wgc.doCapture();
}