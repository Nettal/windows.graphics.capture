//
// Created by Nettal on 2024/5/26.
//

#include <iostream>
#include "FrameSender.h"
#include "lz4.h"

void FrameSender::waitRequireSlot(const FrameSender::SlotSupplier &supplier) {
    DXGIMapping available{};
    compressFinished.wait_dequeue(available);
    if (enableDebugCheck)
        std::cerr << "copy" << "\n";
    compressWaiting.enqueue(supplier(available));
    checkQueueSize();
}

FrameSender::FrameSender(const std::vector<DXGIMapping> &slot) : compressWaiting{slot.size()},
                                                                 compressFinished{slot.size()},
                                                                 sendWaiting(slot.size()),
                                                                 sendFinished(slot.size()),
                                                                 globalQueueSize(slot.size()) {
    compressFinished.enqueue_bulk(slot.begin(), slot.size());
    std::vector<BufferHolder<CompressedFrame>> buffers{slot.size()};
    sendFinished.enqueue_bulk(buffers.begin(), buffers.size());
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
    BufferHolder<CompressedFrame> bufferHolder{};
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
    bufferHolder.extra = {mapping.frameDesc.Width, mapping.frameDesc.Height, mapping.mappedRect.Pitch};
    bufferHolder.usedSize = destSize;
    sendWaiting.enqueue(bufferHolder);
    compressFinished.enqueue(mapping);
    checkQueueSize();
}

void FrameSender::sendOp() {
    BufferHolder <CompressedFrame> bufferHolder;
    sendWaiting.wait_dequeue(bufferHolder);
//    std::this_thread::sleep_for(std::chrono::seconds(1));
    if (enableDebugCheck)
        std::cerr << "sendOp, size: " << bufferHolder.usedSize << "\n";
    sendFinished.enqueue(bufferHolder);
    checkQueueSize();
}

void FrameSender::stop() {
    running = 0;
}

void FrameSender::start() {
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
