//
// Created by Nettal on 2024/5/25.
//

#include "FrameProcessor.h"
#include "wgc/WGCCapture.h"
#include "../shared/network.h"

int main() {
    auto server = TheServer(37385);
    auto socket = server.sAccept();
    auto ctx = D3D11Context();
    auto capture = std::make_shared<WGCCapture>(ctx);
    auto sender = std::make_shared<FrameSender>(ctx, capture->currentFrameSize(), socket);
    auto fp = FrameProcessor{ctx, sender, capture};
    fp.doCapture();
}