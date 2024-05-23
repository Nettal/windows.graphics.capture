//
// Created by Nettal on 2024/5/23.
//

#ifndef PARSEC_CALLBACKIMPL_H
#define PARSEC_CALLBACKIMPL_H

#include <rpc.h>
#include "win/windows.graphics.capture.h"

struct ImplComCallback;

#define CDirect3D11CaptureFramePool_IInspectable __FITypedEventHandler_2_Windows__CGraphics__CCapture__CDirect3D11CaptureFramePool_IInspectable

typedef void (*ImplCallback)(
        struct ImplComCallback *This,
        __x_ABI_CWindows_CGraphics_CCapture_CIDirect3D11CaptureFramePool *sender,
        IInspectable *args);

typedef struct ImplComCallback {
    CONST_VTBL struct __FITypedEventHandler_2_Windows__CGraphics__CCapture__CDirect3D11CaptureFramePool_IInspectableVtbl *lpVtbl;
    void *userPtr;
    ImplCallback callback;
    LONG ref_count;
    IID iid_IUnknown;
    IID iid_ITypedEventHandle;
    struct __FITypedEventHandler_2_Windows__CGraphics__CCapture__CDirect3D11CaptureFramePool_IInspectableVtbl vTableSpace;
} ImplComCallback;

CDirect3D11CaptureFramePool_IInspectable *createInspectable(ImplCallback callback, void *userPtr);


#endif //PARSEC_CALLBACKIMPL_H
