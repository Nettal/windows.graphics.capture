//
// Created by Nettal on 2024/5/26.
//

#ifndef WGC_FRAMESENDER_H
#define WGC_FRAMESENDER_H

#include <lz4.h>
#include "./concurrent/blockingconcurrentqueue.h"
#include "capture/wgc/windows_graphics_capture.h"
#include "functional"
#include "DXGIMapping.h"
#include "D3D11Context.h"
#include "shared/network.h"
#include "AbstractCapture.h"
#include "shared/shared.h"

using moodycamel::BlockingConcurrentQueue;

class FrameSender {
    using SlotSupplier = std::function<DXGIMapping &(DXGIMapping &available)>;

    template<typename T>
    struct BufferHolder {
        void *buffer{};
        void *userPtr{};
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

    using FrameBuffer = BufferHolder<IMAGE_TYPE>;
    using CompressOp = std::function<void(DXGIMapping &mapping, FrameBuffer &frameBuffer)>;
    BlockingConcurrentQueue<DXGIMapping> compressFinished{};
    BlockingConcurrentQueue<DXGIMapping> compressWaiting{};
    BlockingConcurrentQueue<FrameBuffer> sendFinished{};
    BlockingConcurrentQueue<FrameBuffer> sendWaiting{};
    std::thread compressThread{};
    std::thread sendThread{};
    int running{};
    static constexpr bool enableDebugCheck = false;
    size_t globalQueueSize{};
    int64_t socket{};
    std::shared_ptr<D3D11Context> ctx{};
    int numBuffer{};
    CompressOp compress{};

    void sendOp();

    void compressOp();

public:

    void waitRequireSlot(const SlotSupplier &supplier);

    void preCapture(AbstractCapture *capture);

    void endCapture(AbstractCapture *capture);

    explicit FrameSender(std::shared_ptr<D3D11Context> ctx, CompressOp compress, int64_t socket, int numBuffer = 2);

    explicit FrameSender() = default;

    void checkQueueSize();

    static const CompressOp lz4Compress;
    static const CompressOp jpegTurboCompress;
};

#endif //WGC_FRAMESENDER_H
