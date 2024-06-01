//
// Created by Nettal on 2024/5/28.
//

#ifndef WGC_CAPTURESHARED_H
#define WGC_CAPTURESHARED_H

#include <d3d11.h>
#include <vector>

typedef struct SIZE2D {
    int32_t width;
    int32_t height;
} SIZE2D;

typedef struct OnFrameArriveParameter {
    ID3D11Texture2D *d3d11Texture2D;
    uint64_t systemRelativeTime;
    SIZE2D frameSize;
    RECT *dirties;
    uint32_t dirtiesNum;
} OnFrameArriveParameter;
#endif //WGC_CAPTURESHARED_H
