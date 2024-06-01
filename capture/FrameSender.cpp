//
// Created by Nettal on 2024/5/26.
//

#include <iostream>
#include <utility>
#include "FrameSender.h"
#include "lz4.h"
#include "shared/shared.h"
#include <turbojpeg.h>

void FrameSender::waitRequireSlot(const FrameSender::SlotSupplier &supplier) {
    DXGIMapping available{};
    compressFinished.wait_dequeue(available);
    if (enableDebugCheck)
        std::cerr << "copy" << "\n";
    compressWaiting.enqueue(supplier(available));
    checkQueueSize();
}

void FrameSender::checkQueueSize() {
    if (!enableDebugCheck)
        return;
    assert(sendWaiting.size_approx() <= globalQueueSize);
    assert(sendFinished.size_approx() <= globalQueueSize);
    assert(compressWaiting.size_approx() <= globalQueueSize);
    assert(compressFinished.size_approx() <= globalQueueSize);
}

void FrameSender::compressOp() {
    DXGIMapping mapping{};
    FrameBuffer bufferHolder{};
    compressWaiting.wait_dequeue(mapping);
    sendFinished.wait_dequeue(bufferHolder);
    if (enableDebugCheck)
        fprintf(stderr, "compressOp, rect{%d,%d}", mapping.frameDesc.Width, mapping.frameDesc.Height);
    compress(mapping, bufferHolder);
    sendWaiting.enqueue(bufferHolder);
    compressFinished.enqueue(mapping);
    checkQueueSize();
}

void FrameSender::sendOp() {
    FrameBuffer bufferHolder;
    sendWaiting.wait_dequeue(bufferHolder);
//    std::this_thread::sleep_for(std::chrono::seconds(1));
    if (enableDebugCheck)
        std::cerr << "sendOp, size: " << bufferHolder.usedSize << "\n";
    mw_send(socket, &bufferHolder.extra, sizeof(IMAGE_TYPE));
    mw_send(socket, bufferHolder.buffer, bufferHolder.usedSize);
    sendFinished.enqueue(bufferHolder);
    checkQueueSize();
}

FrameSender::FrameSender(std::shared_ptr<D3D11Context> ctx, CompressOp compress, int64_t socket, int numBuffer)
        : ctx(std::move(ctx)), socket(socket), numBuffer(numBuffer), compress(std::move(compress)),
          compressWaiting{static_cast<size_t>(numBuffer)},
          compressFinished{static_cast<size_t>(numBuffer)},
          sendWaiting(static_cast<size_t>(numBuffer)),
          sendFinished(static_cast<size_t>(numBuffer)),
          globalQueueSize(static_cast<size_t>(numBuffer)) {
    std::vector<FrameBuffer> buffers{static_cast<size_t>(numBuffer)};
    sendFinished.enqueue_bulk(buffers.begin(), buffers.size());
}

void FrameSender::preCapture(AbstractCapture *capture) {
    for (int i = 0; i < numBuffer; ++i) {
        compressFinished.enqueue(DXGIMapping{ctx->d3d11Device, capture->currentFrameSize(), ctx->deviceCtx});
    }
    running = 1;
    compressThread = std::thread{[this]() {
        while (running)
            compressOp();
    }};
    sendThread = std::thread{[this]() {
        while (running)
            sendOp();
    }};
}

void FrameSender::endCapture(AbstractCapture *capture) {
    running = 0;
}

const FrameSender::CompressOp FrameSender::lz4Compress = [](DXGIMapping &mapping,
                                                            FrameSender::FrameBuffer &frameBuffer) {
    auto bufferSize = mapping.frameDesc.Height * mapping.mappedRect.Pitch;
    if (frameBuffer.userPtr == nullptr) {
        frameBuffer.userPtr = new std::vector<char>(bufferSize);
    }
    if (bufferSize > frameBuffer.byteSize) {
        frameBuffer.reSize(bufferSize);
        reinterpret_cast<std::vector<char> *>(frameBuffer.userPtr)->resize(bufferSize);
    }
    frameBuffer.extra = {static_cast<uint32_t>(mapping.dataRect.left),
                         static_cast<uint32_t>(mapping.dataRect.top),
                         static_cast<uint32_t>(mapping.dataRect.right - mapping.dataRect.left),
                         static_cast<uint32_t>(mapping.dataRect.bottom - mapping.dataRect.top), 0};
    if (frameBuffer.extra.h != mapping.frameDesc.Height
        || frameBuffer.extra.w != mapping.frameDesc.Width) { // ranged, do mem copy
        auto dest = *reinterpret_cast<std::vector<char> *>(frameBuffer.userPtr);
        for (int i = 0; i < frameBuffer.extra.h; ++i) {
            auto destPtr = &dest[i * frameBuffer.extra.w * 4]; // 4 channel
            auto srcPtr = ((char *) mapping.mappedRect.pBits)
                          + mapping.dataRect.left * 4 // current line start
                          + mapping.mappedRect.Pitch * i * 4; // pre full lines
            memcpy(destPtr, srcPtr, frameBuffer.extra.w * 4);
        }
        frameBuffer.extra.size = LZ4_compress_default((char *) dest.data(),
                                                      (char *) frameBuffer.buffer,
                                                      bufferSize,
                                                      bufferSize);
    } else {
        frameBuffer.extra.size = LZ4_compress_default((char *) mapping.mappedRect.pBits,
                                                      (char *) frameBuffer.buffer,
                                                      bufferSize,
                                                      bufferSize);
    }
    assert(frameBuffer.extra.size != 0);
    frameBuffer.usedSize = frameBuffer.extra.size;
};

const FrameSender::CompressOp FrameSender::jpegTurboCompress = [](DXGIMapping &mapping, FrameBuffer &frameBuffer) {

};