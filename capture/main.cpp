//
// Created by Nettal on 2024/5/25.
//

#include "FrameProcessor.h"
#include "wgc/WGCCapture.h"

int main() {
    auto server = TheServer(37385);
    auto socket = server.sAccept();
    auto ctx = std::make_shared<D3D11Context>();
    auto sender = std::make_shared<FrameSender>(ctx, socket);
    auto fp = std::make_shared<FrameProcessor>(ctx, sender);
    auto capture = std::make_shared<WGCCapture>(ctx, fp);
    capture->start();
}