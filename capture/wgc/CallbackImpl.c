//
// Created by Nettal on 2024/5/23.
//

#include "CallbackImpl.h"
#include "IIDUtils.h"

__stdcall HRESULT callback_impl_QueryInterface(
        __FITypedEventHandler_2_Windows__CGraphics__CCapture__CDirect3D11CaptureFramePool_IInspectable *This,
        REFIID riid,
        void **ppvObject) {
    ImplComCallback *this_ = ((ImplComCallback *) This);
    // check to see what interface has been requested
    if (IsEqualGUID(riid, &this_->iid_IUnknown) || IsEqualGUID(riid, &this_->iid_ITypedEventHandle) ) {
        InterlockedIncrement(&this_->ref_count);
        *ppvObject = This;
        return S_OK;
    } else {
        *ppvObject = 0;
        return E_NOINTERFACE;
    }
}

__stdcall ULONG
callback_impl_AddRef(__FITypedEventHandler_2_Windows__CGraphics__CCapture__CDirect3D11CaptureFramePool_IInspectable *This) {
    ImplComCallback *this_ = ((ImplComCallback *) This);
    return InterlockedIncrement(&this_->ref_count);
}

__stdcall ULONG callback_impl_Release(
        __FITypedEventHandler_2_Windows__CGraphics__CCapture__CDirect3D11CaptureFramePool_IInspectable *This) {
    ImplComCallback *this_ = ((ImplComCallback *) This);
    // decrement object reference count
    LONG count = InterlockedDecrement(&this_->ref_count);

    if (count == 0) {
        free(this_);
        return 0;
    } else {
        return count;
    }
}

__stdcall HRESULT
callback_impl_Invoke(__FITypedEventHandler_2_Windows__CGraphics__CCapture__CDirect3D11CaptureFramePool_IInspectable *This,
                     __x_ABI_CWindows_CGraphics_CCapture_CIDirect3D11CaptureFramePool *sender,
                     IInspectable *args) {
    ImplComCallback *this_ = ((ImplComCallback *) This);
    this_->callback(this_, sender, args);
    return S_OK;
}

CDirect3D11CaptureFramePool_IInspectable *
callback_impl_createInspectable(ImplCallback callback, void *userPtr) {
    struct ImplComCallback *result = calloc(sizeof(struct ImplComCallback), 1);
    result->lpVtbl = &result->vTableSpace;
    result->lpVtbl->AddRef = callback_impl_AddRef;
    result->lpVtbl->QueryInterface = callback_impl_QueryInterface;
    result->lpVtbl->Release = callback_impl_Release;
    result->lpVtbl->Invoke = callback_impl_Invoke;
    result->userPtr = userPtr;
    result->callback = callback;
    result->ref_count = 1;
    result->iid_IUnknown = iid_utils_guidFrom("00000000-0000-0000-C000-000000000046");
    result->iid_ITypedEventHandle = iid_utils_guidFrom("51A947F7-79CF-5A3E-A3A5-1289CFA6DFE8");
    return result;
}
