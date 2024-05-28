//
// Created by Nettal on 2024/5/26.
//

#ifndef WGC_FRAMESENDER_H
#define WGC_FRAMESENDER_H

#include "./concurrent/blockingconcurrentqueue.h"
#include "capture/wgc/windows_graphics_capture.h"
#include "functional"
#include "DXGIMapping.h"
#include "D3D11Context.h"
#include "shared/network.h"

using moodycamel::BlockingConcurrentQueue;

class FrameSender {
    using SlotSupplier = std::function<DXGIMapping &(DXGIMapping &available)>;

    struct CompressedFrame {
        uint32_t w;
        uint32_t h;
    };

    template<typename T>
    struct BufferHolder {
        void *buffer{};
        int64_t byteSize{};
        int64_t usedSize{};
        T extra;

        explicit BufferHolder() = default;

        explicit BufferHolder(int64_t byteSize) {
            buffer = calloc(1, byteSize);
        }

        void freeBuffer() {
            free(buffer);
            buffer = nullptr;
        }

        void reSize(int64_t newSize) {
            buffer = realloc(buffer, newSize);
        }
    };

    using FrameBuffer = BufferHolder<CompressedFrame>;
    BlockingConcurrentQueue<DXGIMapping> compressFinished{};
    BlockingConcurrentQueue<DXGIMapping> compressWaiting{};
    BlockingConcurrentQueue<FrameBuffer> sendFinished{};
    BlockingConcurrentQueue<FrameBuffer> sendWaiting{};
    std::thread compressThread{};
    std::thread sendThread{};
    int running{};
    static constexpr bool enableDebugCheck = false;
    size_t globalQueueSize{};
    std::shared_ptr<TheServer> socket;

    void sendOp();

    void compressOp();

    FrameSender(const std::initializer_list<DXGIMapping> &slot, std::shared_ptr<TheServer> socket);

public:

    void waitRequireSlot(const SlotSupplier &supplier);

    void stop();

    void start();

    explicit FrameSender(const D3D11Context &ctx, SIZE2D frameSize, std::shared_ptr<TheServer> socket);

    explicit FrameSender() = default;

    void checkQueueSize();
};

#endif //WGC_FRAMESENDER_H
