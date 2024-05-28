//
// Created by Nettal on 2024/5/26.
//

#include <iostream>
#include <utility>
#include "FrameSender.h"
#include "lz4.h"
#include "shared/shared.h"

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
    auto bufferSize = mapping.frameDesc.Height * mapping.mappedRect.Pitch;
    if (bufferSize > bufferHolder.byteSize) {
        bufferHolder.reSize(bufferSize);
    }
    auto destSize = LZ4_compress_default((char *) mapping.mappedRect.pBits, (char *) bufferHolder.buffer, bufferSize,
                                         bufferSize);
    assert(destSize != 0);
    bufferHolder.extra = {mapping.frameDesc.Width, mapping.frameDesc.Height};
    bufferHolder.usedSize = destSize;
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
    IMAGE_TYPE image{bufferHolder.extra.w, bufferHolder.extra.h, bufferHolder.usedSize};
    mw_send(socket, &image, sizeof(IMAGE_TYPE));
    mw_send(socket, bufferHolder.buffer, bufferHolder.usedSize);
    sendFinished.enqueue(bufferHolder);
    checkQueueSize();
}

FrameSender::FrameSender(std::shared_ptr<D3D11Context> ctx, int64_t socket, int numBuffer)
        : ctx(std::move(ctx)), socket(socket), numBuffer(numBuffer),
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
