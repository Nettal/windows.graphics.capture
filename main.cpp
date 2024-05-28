//
// Created by Nettal on 2024/5/25.
//

#include "main.h"
#include "capture/WindowsGraphicsCapture.h"
#include "capture/wgc/WGCCapture.h"
#include "shared/network.h"

int main() {
    auto server = std::make_shared<TheServer>(37385);
    server->sAccept();
    auto ctx = D3D11Context();
    auto capture = std::make_shared<WGCCapture>(ctx);
    auto sender = std::make_shared<FrameSender>(ctx, capture->currentFrameSize(), server);
    auto fp = FrameProcessor{ctx, sender, capture};
    fp.doCapture();
}