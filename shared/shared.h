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
} IMAGE_TYPE;

#endif //CAPTURE_SHARED_H
