//
// Created by Nettal on 2024/5/22.
//

#ifdef __cplusplus
extern "C"{
#endif

#include "stdint.h"

typedef struct WGC_SIZE2D {
    int32_t width;
    int32_t height;
} WGC_SIZE2D;

typedef struct OnFrameArriveParameter {
    ID3D11Texture2D *d3d11Texture2D;
    ID3D11Device *d3D11Device;
    int64_t systemRelativeTime;
    WGC_SIZE2D surfaceSize; // not really accurate, use mapped texture's description
    void *userPtr;
} OnFrameArriveParameter;
typedef struct OnFrameArriveRet {
    int running;
} OnFrameArriveRet;

typedef void (*OnFrameArrive)(OnFrameArriveParameter *onFrameArriveParameter, OnFrameArriveRet *ret);

void *
wgc_initial_everything(HMONITOR monitorToCapture, WGC_SIZE2D *frameSize, ID3D11Device *d3d11Device,
                       OnFrameArrive frameArrive, void *userPtr);

void wgc_do_capture_on_this_thread(void *);

#ifdef __cplusplus
}
#endif