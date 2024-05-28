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
#include "CaptureShared.h"

typedef void (*WgcOnFrameArrive)(OnFrameArriveParameter *onFrameArriveParameter, OnFrameArriveRet *ret);

void *wgc_initial_everything(HMONITOR monitorToCapture, SIZE2D *frameSize, ID3D11Device *d3d11Device);

void wgc_do_capture_on_this_thread(void *, WgcOnFrameArrive frameArrive, void *userPtr);

#endif
#ifdef __cplusplus
}
#endif