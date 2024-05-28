//
// Created by Nettal on 2024/5/28.
//

#ifndef WGC_CAPTURESHARED_H
#define WGC_CAPTURESHARED_H
#ifdef __cplusplus
extern "C" {
#endif
#include <d3d11.h>
#include <stdint.h>

typedef struct SIZE2D {
    int32_t width;
    int32_t height;
} SIZE2D;

typedef struct OnFrameArriveParameter {
    ID3D11Texture2D *d3d11Texture2D;
    ID3D11Device *d3D11Device;
    uint64_t systemRelativeTime;
    SIZE2D frameSize;
    void *userPtr;
} OnFrameArriveParameter;
typedef struct OnFrameArriveRet {
    int running;
} OnFrameArriveRet;
#ifdef __cplusplus
};
#endif
#endif //WGC_CAPTURESHARED_H
