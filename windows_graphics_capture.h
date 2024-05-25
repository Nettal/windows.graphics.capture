//
// Created by Nettal on 2024/5/22.
//

#ifdef __cplusplus
extern "C"{
#endif

#include "stdint.h"

typedef struct OnFrameArriveParameter {
    ID3D11Texture2D *d3d11Texture2D;
    ID3D11Device *d3D11Device;
    int32_t surfaceWidth;
    int32_t surfaceHeight;
    void *userPtr;
} OnFrameArriveParameter;
typedef struct OnFrameArriveRet {
    int running;
} OnFrameArriveRet;

typedef void (*OnFrameArrive)(OnFrameArriveParameter *onFrameArriveParameter,OnFrameArriveRet *ret);

void *wgc_initial_everything(OnFrameArrive frameArrive, ID3D11Device *d3d11Device, void *userPtr);

void wgc_do_capture_on_this_thread(void *);

#ifdef __cplusplus
}
#endif