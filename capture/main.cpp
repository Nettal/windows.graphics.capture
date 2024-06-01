//
// Created by Nettal on 2024/5/25.
//

#include "FrameProcessor.h"
#include "wgc/WGCCapture.h"
#include "capture/wdd/DesktopDuplication.h"

int main() {
    auto server = TheServer(37385);
    auto socket = server.sAccept();
    auto ctx = std::make_shared<D3D11Context>();
    auto sender = std::make_shared<FrameSender>(ctx, FrameSender::lz4Compress, socket);
    auto fp = std::make_shared<FrameProcessor>(ctx, sender, false);
    auto capture = std::make_shared<DesktopDuplication>(ctx, fp);
    capture->start();
}