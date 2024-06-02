//
// Created by Nettal on 2024/5/28.
//

#ifndef CAPTURE_SHARED_H
#define CAPTURE_SHARED_H

#include <stdint.h>

typedef struct {
    uint32_t x;
    uint32_t y;
    uint32_t w;
    uint32_t h;
    int64_t size;
    uint64_t flags; // have next when not zero
} IMAGE_TYPE;

typedef struct {
    int64_t size;
    uint64_t flags; // have next when not zero
} IMAGE_TYPE_NEXT;
#endif //CAPTURE_SHARED_H
