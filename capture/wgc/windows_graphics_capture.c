//
// Created by Nettal on 2024/5/22.
//
#include "roapi.h"
#include <rpc.h>
#include <stdio.h>
#include <assert.h>
#include <d3d11.h>
#include "winstring.h"
#include "windows.graphics.capture.interop.h"
#include "capture/wgc/include/windows.graphics.capture.h"
#include "capture/wgc/include/windows.graphics.directx.direct3d11.interop.h"
#include "IIDUtils.h"
#include "CallbackImpl.h"
#include "IDirect3DDxgiInterfaceAccess.h"
#include "windows_graphics_capture.h"

#define getPFN(x) (typeof(&x)) GetProcAddress(LoadLibraryA("combase.dll"), #x)

typedef __x_ABI_CWindows_CGraphics_CCapture_CIGraphicsCaptureItem CIGraphicsCaptureItem;
typedef __x_ABI_CWindows_CGraphics_CCapture_CIDirect3D11CaptureFramePoolStatics CDirect3D11CaptureFramePoolStatics;
typedef __x_ABI_CWindows_CGraphics_CCapture_CIDirect3D11CaptureFramePool CIDirect3D11CaptureFramePool;
typedef __x_ABI_CWindows_CGraphics_CCapture_CIGraphicsCaptureSession CIGraphicsCaptureSession;
typedef __x_ABI_CWindows_CGraphics_CCapture_CIDirect3D11CaptureFrame CFrame;
typedef __x_ABI_CWindows_CGraphics_CDirectX_CDirect3D11_CIDirect3DSurface CIDirect3DSurface;
typedef __x_ABI_CWindows_CGraphics_CSizeInt32 CSizeInt32;
typedef __x_ABI_CWindows_CGraphics_CDirectX_CDirect3D11_CIDirect3DDevice CIDirect3DDevice;
typedef __x_ABI_CWindows_CGraphics_CDirectX_CDirect3D11_CDirect3DSurfaceDescription CDirect3DSurfaceDescription;
typedef __x_ABI_CWindows_CFoundation_CTimeSpan CTimeSpan;

IGraphicsCaptureItemInterop *graphicsCaptureItemInteropFunc() {
    static IGraphicsCaptureItemInterop *iGraphicsCaptureItemInterop = NULL;
    if (iGraphicsCaptureItemInterop != NULL)
        return iGraphicsCaptureItemInterop;
    typeof(&RoGetActivationFactory) activePtr = getPFN(RoGetActivationFactory);
    typeof(&WindowsCreateString) strPtr = getPFN(WindowsCreateString);
    typeof(&WindowsDeleteString) delStrPtr = getPFN(WindowsDeleteString);

    HSTRING str;
    strPtr(RuntimeClass_Windows_Graphics_Capture_GraphicsCaptureItem,
           sizeof(RuntimeClass_Windows_Graphics_Capture_GraphicsCaptureItem) /
           sizeof(wchar_t) - 1, &str);
    // IGraphicsCaptureItemInterop
    GUID GUID_GraphicsCaptureItemInterop = iid_utils_guidFrom("3628e81b-3cac-4c60-b7f4-23ce0e0c3356");
    HRESULT ret = activePtr(str, &GUID_GraphicsCaptureItemInterop, &iGraphicsCaptureItemInterop);
    delStrPtr(str);
    assert(ret == S_OK);
    assert(iGraphicsCaptureItemInterop != NULL);
    return iGraphicsCaptureItemInterop;
}

CDirect3D11CaptureFramePoolStatics *direct3D11CaptureFramePoolStaticsFunc() {
    static CDirect3D11CaptureFramePoolStatics *funcTable = NULL;
    if (funcTable != NULL)
        return funcTable;
    typeof(&RoGetActivationFactory) activePtr = getPFN(RoGetActivationFactory);
    typeof(&WindowsCreateString) strPtr = getPFN(WindowsCreateString);
    typeof(&WindowsDeleteString) delStrPtr = getPFN(WindowsDeleteString);

    HSTRING str;
    strPtr(RuntimeClass_Windows_Graphics_Capture_Direct3D11CaptureFramePool,
           sizeof(RuntimeClass_Windows_Graphics_Capture_Direct3D11CaptureFramePool) /
           sizeof(wchar_t) - 1, &str);
    // IDirect3D11CaptureFramePoolStatics
    GUID guid = iid_utils_guidFrom("7784056a-67aa-4d53-ae54-1088d5a8ca21");
    HRESULT ret = activePtr(str, &guid, &funcTable);
    delStrPtr(str);
    assert(ret == S_OK);
    assert(funcTable != NULL);
    return funcTable;
}

struct WGC_SurfaceTranslate {
    GUID GUID_IDirect3DDxgiInterfaceAccess;
    GUID GUID_IDXGISurface;
    GUID GUID_ID3D11Texture2D;
    int running;
    ID3D11Device *d3d11Device;
    SIZE2D framePoolSize;
    CIDirect3DDevice *ciDirect3DDevice;
    int numberOfBuffers;
    WgcOnFrameArrive frameArrive;
    void *userPtr;
};;

void wgc_onFrameArrive(struct ImplComCallback *This,
                       __x_ABI_CWindows_CGraphics_CCapture_CIDirect3D11CaptureFramePool *framePool,
                       IInspectable *args) {
    CFrame *frame;
    framePool->lpVtbl->TryGetNextFrame(framePool, &frame);
    CTimeSpan systemRelativeTime;
    frame->lpVtbl->get_SystemRelativeTime(frame, &systemRelativeTime);
    CSizeInt32 frameSize;
    frame->lpVtbl->get_ContentSize(frame, &frameSize);
    CIDirect3DSurface *surface;
    frame->lpVtbl->get_Surface(frame, &surface);
    CDirect3DSurfaceDescription surfaceDescription;
    surface->lpVtbl->get_Description(surface, &surfaceDescription);
    struct WGC_SurfaceTranslate *surfaceTranslate = This->userPtr;
    // fix incorrect pool size
    if (frameSize.Height != surfaceTranslate->framePoolSize.height ||
        frameSize.Width != surfaceTranslate->framePoolSize.width) {
        surfaceTranslate->framePoolSize.height = frameSize.Height;
        surfaceTranslate->framePoolSize.width = frameSize.Width;
        framePool->lpVtbl->Recreate(framePool, surfaceTranslate->ciDirect3DDevice,
                                    surfaceDescription.Format, surfaceTranslate->numberOfBuffers,
                                    *(CSizeInt32 *) &surfaceTranslate->framePoolSize);
        surface->lpVtbl->Release(surface);
        frame->lpVtbl->Release(frame);
        return;
    }
    // skip incorrect surface
    if (frameSize.Height != surfaceDescription.Height ||
        frameSize.Width != surfaceDescription.Width) {
        surface->lpVtbl->Release(surface);
        frame->lpVtbl->Release(frame);
        return;
    }
    CIDirect3DDxgiInterfaceAccess *dDxgiInterfaceAccess;
    surface->lpVtbl->QueryInterface(surface, &surfaceTranslate->GUID_IDirect3DDxgiInterfaceAccess,
                                    &dDxgiInterfaceAccess);
    ID3D11Texture2D *texture2D;
    dDxgiInterfaceAccess->lpVtbl->GetInterface(dDxgiInterfaceAccess, &surfaceTranslate->GUID_ID3D11Texture2D,
                                               &texture2D);
    // outer
    //fprintf(stderr, "surface{%d,%d}\n", surfaceDescription.Width, surfaceDescription.Height);
    //fprintf(stderr, "frame{%d,%d}\n", frameSize.Width, frameSize.Height);

    OnFrameArriveParameter parameter = {texture2D,
                                        surfaceTranslate->d3d11Device,
                                        systemRelativeTime.Duration,
                                        frameSize.Width,
                                        frameSize.Height,
                                        surfaceTranslate->userPtr};
    OnFrameArriveRet arriveRet = {surfaceTranslate->running};
    assert(surfaceTranslate->frameArrive);
    surfaceTranslate->frameArrive(&parameter, &arriveRet);
    // outer end
    surfaceTranslate->running = arriveRet.running;

    texture2D->lpVtbl->Release(texture2D);
    dDxgiInterfaceAccess->lpVtbl->Release(dDxgiInterfaceAccess);
    surface->lpVtbl->Release(surface);
    frame->lpVtbl->Release(frame); // release to be able to get next frame
}

#define CHECK_RESULT_OR_RET(x) do{if(FAILED(x)) {fprintf(stderr,"error at %s:%d\n",__FILE__,__LINE__); return 0;}} while(0)
#define CHECK_RESULT(x) do{if(FAILED(x)) {fprintf(stderr,"error at %s:%d\n",__FILE__,__LINE__); return;}} while(0)

typedef struct WGC_INTERNAL_FILED {
    struct WGC_SurfaceTranslate surfaceTranslate;
    CIGraphicsCaptureSession *captureSession;
    CIDirect3D11CaptureFramePool *framePool;
    CIGraphicsCaptureItem *CaptureItem;
    IDXGIDevice *dxgiDevice;
    IInspectable *d3dInspectable;
    CIDirect3DDevice *ciDirect3DDevice;
    CDirect3D11CaptureFramePool_IInspectable *fpInspectable;
    ID3D11Device *d3d11Device;
} WGC_INTERNAL_FILED;

void wgc_do_capture_on_this_thread(void *field, WgcOnFrameArrive frameArrive, void *userPtr) {
    WGC_INTERNAL_FILED *wif = field;
    wif->surfaceTranslate.frameArrive = frameArrive;
    wif->surfaceTranslate.userPtr = userPtr;
    HRESULT ret = wif->captureSession->lpVtbl->StartCapture(wif->captureSession);
    CHECK_RESULT(ret);
    MSG msg = {};
    while (GetMessageW(&msg, NULL, 0, 0)) {
        if (!wif->surfaceTranslate.running) {
            wif->captureSession->lpVtbl->Release(wif->captureSession);
            wif->framePool->lpVtbl->Release(wif->framePool);
            wif->CaptureItem->lpVtbl->Release(wif->CaptureItem);
            wif->ciDirect3DDevice->lpVtbl->Release(wif->ciDirect3DDevice);
            wif->fpInspectable->lpVtbl->Release(wif->fpInspectable);
            wif->dxgiDevice->lpVtbl->Release(wif->dxgiDevice);
            OleUninitialize();
        }
        TranslateMessage(&msg);// onFrameArrive loop thread
        DispatchMessageW(&msg);
    }
}

void *
wgc_initial_everything(HMONITOR monitorToCapture, SIZE2D *frameSize, ID3D11Device *d3d11Device) {
    WGC_INTERNAL_FILED *wif = calloc(sizeof(struct WGC_INTERNAL_FILED), 1);
    wif->d3d11Device = d3d11Device;
    CoInitialize(0);
    IID GUID_IGraphicsCaptureItem = iid_utils_guidFrom("79c3f95b-31f7-4ec2-a464-632ef5d30760");
    HRESULT ret = graphicsCaptureItemInteropFunc()->lpVtbl->CreateForMonitor(graphicsCaptureItemInteropFunc(),
                                                                             monitorToCapture,
                                                                             &GUID_IGraphicsCaptureItem,
                                                                             &wif->CaptureItem);
    CHECK_RESULT_OR_RET(ret);
    wif->CaptureItem->lpVtbl->get_Size(wif->CaptureItem, frameSize);


    GUID dxgiUID = iid_utils_guidFrom("54ec77fa-1377-44e6-8c32-88fd5f44c84c");
    ret = d3d11Device->lpVtbl->QueryInterface(d3d11Device, &dxgiUID, &wif->dxgiDevice);
    CHECK_RESULT_OR_RET(ret);
    CreateDirect3D11DeviceFromDXGIDevice(wif->dxgiDevice, &wif->d3dInspectable);
    GUID winrtD3DDUID = iid_utils_guidFrom("a37624ab-8d5f-4650-9d3e-9eae3d9bc670");
    wif->d3dInspectable->lpVtbl->QueryInterface(wif->d3dInspectable, &winrtD3DDUID, &wif->ciDirect3DDevice);
    wif->surfaceTranslate = (struct WGC_SurfaceTranslate) {
            iid_utils_guidFrom("A9B3D012-3DF2-4EE3-B8D1-8695F457D3C1"),
            iid_utils_guidFrom("cafcb56c-6ac3-4889-bf47-9e23bbd260ec"),
            iid_utils_guidFrom("6f15aaf2-d208-4e89-9ab4-489535d34f9c"),
            1, d3d11Device, *frameSize, wif->ciDirect3DDevice,
            2, 0, 0};
    // maybe try CreateFreeThreaded
    ret = direct3D11CaptureFramePoolStaticsFunc()->lpVtbl->Create(direct3D11CaptureFramePoolStaticsFunc(),
                                                                  wif->ciDirect3DDevice,
                                                                  DirectXPixelFormat_B8G8R8A8UIntNormalized,
                                                                  wif->surfaceTranslate.numberOfBuffers,
                                                                  *(CSizeInt32 *) frameSize, &wif->framePool);
    CHECK_RESULT_OR_RET(ret);
    wif->framePool->lpVtbl->CreateCaptureSession(wif->framePool, wif->CaptureItem, &wif->captureSession);
    wif->fpInspectable = callback_impl_createInspectable(wgc_onFrameArrive,
                                                         &wif->surfaceTranslate);
    EventRegistrationToken token;
    ret = wif->framePool->lpVtbl->add_FrameArrived(wif->framePool, wif->fpInspectable, &token);
    CHECK_RESULT_OR_RET(ret);
    return wif;
}