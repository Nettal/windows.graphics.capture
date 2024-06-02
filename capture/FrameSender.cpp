//
// Created by Nettal on 2024/5/26.
//

#include <iostream>
#include <utility>
#include "FrameSender.h"
#include "lz4.h"
#include "shared/shared.h"
#include <turbojpeg.h>
#include "../concurrent/BS_thread_pool.hpp"

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
    bufferHolder.send(bufferHolder, socket);
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

constexpr bool THREADING = true;
const FrameSender::CompressOp FrameSender::lz4Compress = [](DXGIMapping &mapping,
                                                            FrameSender::FrameBuffer &frameBuffer) {
    auto bufferSize = mapping.frameDesc.Height * mapping.mappedRect.Pitch;
    if (mapping.userPtr == nullptr) {
        mapping.userPtr = new BufferHolder<char>(bufferSize);
    }
    auto *pHolder = reinterpret_cast<BufferHolder <IMAGE_TYPE_NEXT> *>(mapping.userPtr);
    if (bufferSize > frameBuffer.byteSize) {
        frameBuffer.reSize(bufferSize);
        pHolder->reSize(bufferSize);
    }
    frameBuffer.send = [](FrameBuffer &bufferHolder, int64_t socket_) {
        mw_send(socket_, &bufferHolder.extra, sizeof(IMAGE_TYPE));
        mw_send(socket_, bufferHolder.buffer, bufferHolder.usedSize);
    };
    frameBuffer.extra = {static_cast<uint32_t>(mapping.dataRect.left),
                         static_cast<uint32_t>(mapping.dataRect.top),
                         static_cast<uint32_t>(mapping.dataRect.right - mapping.dataRect.left),
                         static_cast<uint32_t>(mapping.dataRect.bottom - mapping.dataRect.top), 0};
    if (frameBuffer.extra.h != mapping.frameDesc.Height
        || frameBuffer.extra.w != mapping.frameDesc.Width) { // ranged, do mem copy
        auto dest = (char *) pHolder->buffer;
        for (int i = 0; i < frameBuffer.extra.h; ++i) {
            auto destPtr = &dest[i * frameBuffer.extra.w * 4]; // 4 channel
            auto srcPtr = ((char *) mapping.mappedRect.pBits)
                          + mapping.dataRect.left * 4 // current line start
                          + mapping.mappedRect.Pitch
                            * (i + frameBuffer.extra.y); // pre full lines
            memcpy(destPtr, srcPtr, frameBuffer.extra.w * 4);
        }
        pHolder->extra.flags = 0;
        frameBuffer.usedSize = frameBuffer.extra.size = LZ4_compress_default((char *) dest,
                                                                             (char *) frameBuffer.buffer,
                                                                             bufferSize,
                                                                             bufferSize);
    }
    if (THREADING) {
        using BS::thread_pool;
        thread_pool threadPool{};
        auto lineCount = mapping.frameDesc.Height / threadPool.get_thread_count();
        auto firstLineCount = mapping.frameDesc.Height % threadPool.get_thread_count() + lineCount;
        threadPool.detach_task([&frameBuffer, &mapping, bufferSize, firstLineCount]() {
            frameBuffer.extra.size = LZ4_compress_default((char *) mapping.mappedRect.pBits,
                                                          (char *) frameBuffer.buffer,
                                                          mapping.mappedRect.Pitch * firstLineCount,
                                                          bufferSize);
        });
        frameBuffer.extra.flags = firstLineCount != mapping.frameDesc.Height;
        auto &currentBuffer = mapping.userPtr;
        for (int i = 1; i < threadPool.get_thread_count(); ++i) {
            if (currentBuffer == nullptr) {
                currentBuffer = new BufferHolder<IMAGE_TYPE_NEXT>(bufferSize);
            }
            auto buffer = reinterpret_cast<BufferHolder <IMAGE_TYPE_NEXT> *>(currentBuffer);
            if (bufferSize > buffer->byteSize) {
                buffer->reSize(bufferSize);
            }
            buffer->extra.flags = (i != threadPool.get_thread_count() - 1);
            threadPool.detach_task([buffer, &mapping, bufferSize, i, lineCount]() {
                buffer->extra.size = buffer->usedSize = LZ4_compress_default(
                        (char *) mapping.mappedRect.pBits
                        + mapping.mappedRect.Pitch * i * lineCount,
                        (char *) buffer->buffer,
                        mapping.mappedRect.Pitch * lineCount,
                        bufferSize);
            });
        }
        threadPool.wait();
        frameBuffer.send = [](FrameBuffer &bufferHolder, int64_t socket_) {
            mw_send(socket_, &bufferHolder.extra, sizeof(IMAGE_TYPE));
            mw_send(socket_, bufferHolder.buffer, bufferHolder.usedSize);
            if (bufferHolder.extra.flags) {
                auto next = static_cast<BufferHolder <IMAGE_TYPE_NEXT> *>(bufferHolder.userPtr);
                assert(next);
                while (next) {
                    mw_send(socket_, &next->extra, sizeof(IMAGE_TYPE_NEXT));
                    mw_send(socket_, next->buffer, next->usedSize);
                    if (next->extra.flags)
                        next = static_cast<BufferHolder <IMAGE_TYPE_NEXT> *>(next->userPtr);
                    else {
                        next = nullptr;
                    }
                }
            }
        };
        return;
    }
    frameBuffer.extra.size = LZ4_compress_default((char *) mapping.mappedRect.pBits,
                                                  (char *) frameBuffer.buffer,
                                                  bufferSize,
                                                  bufferSize);
};

const FrameSender::CompressOp FrameSender::jpegTurboCompress = [](DXGIMapping &mapping, FrameBuffer &frameBuffer) {
    auto bufferSize = tjBufSize(mapping.frameDesc.Width, mapping.frameDesc.Height, TJPF_BGRA);
    if (mapping.userPtr == nullptr) {
        mapping.userPtr = tjInitCompress();
    }
    if (bufferSize > frameBuffer.byteSize) {
        frameBuffer.reSize(bufferSize);
    }
    frameBuffer.extra = {static_cast<uint32_t>(mapping.dataRect.left),
                         static_cast<uint32_t>(mapping.dataRect.top),
                         static_cast<uint32_t>(mapping.dataRect.right - mapping.dataRect.left),
                         static_cast<uint32_t>(mapping.dataRect.bottom - mapping.dataRect.top), 0};
    auto src = (uint8_t *) mapping.mappedRect.pBits
               + frameBuffer.extra.x * 4 // current line start
               + frameBuffer.extra.y * mapping.mappedRect.Pitch; // pre full lines
    auto result = tjCompress2(mapping.userPtr,
                              src,
                              frameBuffer.extra.w,
                              mapping.mappedRect.Pitch, // pitch
                              (int) frameBuffer.extra.h, TJPF_BGRA,
                              (uint8_t **) (&frameBuffer.buffer),
                              (unsigned long *) (&frameBuffer.extra.size), TJSAMP_444, 90,
                              TJFLAG_NOREALLOC | TJFLAG_FASTDCT);
    assert(result == 0);
    assert(frameBuffer.extra.size != 0);
    frameBuffer.usedSize = frameBuffer.extra.size;
};

const FrameSender::CompressOp FrameSender::noOp = [](DXGIMapping &mapping, FrameBuffer &frameBuffer) {
    mw_error("w:%d,h%d", mapping.frameDesc.Width, mapping.frameDesc.Height);
    frameBuffer.reSize(64);
    frameBuffer.extra.size = 64;
    assert(frameBuffer.extra.size != 0);
    frameBuffer.usedSize = frameBuffer.extra.size;
};