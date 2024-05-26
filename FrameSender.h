//
// Created by Nettal on 2024/5/26.
//

#ifndef WGC_FRAMESENDER_H
#define WGC_FRAMESENDER_H

#include "./concurrent/blockingconcurrentqueue.h"
#include "windows_graphics_capture.h"
#include "functional"
#include "DXGIMapping.h"

using moodycamel::BlockingConcurrentQueue;

class FrameSender {
    using SlotSupplier = std::function<DXGIMapping &(DXGIMapping &available)>;

    struct CompressedFrame {
        uint32_t w;
        uint32_t h;
        int32_t pitch;
    };

    template<class T>
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

    BlockingConcurrentQueue<DXGIMapping> compressFinished{};
    BlockingConcurrentQueue<DXGIMapping> compressWaiting{};
    BlockingConcurrentQueue<BufferHolder<CompressedFrame>> sendFinished{};
    BlockingConcurrentQueue<BufferHolder<CompressedFrame>> sendWaiting{};
    std::thread compressThread{};
    std::thread sendThread{};
    int running{};
    static constexpr bool enableDebugCheck = false;
    size_t globalQueueSize{};

    void sendOp();

    void compressOp();


public:
    void waitRequireSlot(const SlotSupplier &supplier);

    void stop();

    void start();

    explicit FrameSender(const std::vector<DXGIMapping> &slot);

    explicit FrameSender() = default;

    void checkQueueSize();
};

#endif //WGC_FRAMESENDER_H
