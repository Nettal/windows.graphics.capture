//
// Created by Nettal on 2024/5/22.
//

#ifdef __cplusplus
extern "C"{
#endif
#ifndef WGC_WINDOWS_GRAPHICS_CAPTURE_H
#define WGC_WINDOWS_GRAPHICS_CAPTURE_H

#include <d3d11.h>
#include "stdint.h"

typedef struct WGCSIZE2D {
    int32_t width;
    int32_t height;
} WGCSIZE2D;
typedef struct WGCOnFrameArriveParameter {
    ID3D11Texture2D *d3d11Texture2D;
    uint64_t systemRelativeTime;
    WGCSIZE2D frameSize;
    void *userPtr;
} WGCOnFrameArriveParameter;
typedef struct WGCOnFrameArriveRet {
    int running;
} WGCOnFrameArriveRet;

typedef void (*WgcOnFrameArrive)(WGCOnFrameArriveParameter *onFrameArriveParameter, WGCOnFrameArriveRet *ret);

void *wgc_initial_everything(HMONITOR monitorToCapture, WGCSIZE2D *frameSize, ID3D11Device *d3d11Device, int bufferNum);

void wgc_do_capture_on_this_thread(void *, WgcOnFrameArrive frameArrive, void *userPtr);

#endif
#ifdef __cplusplus
}
#endif