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
#include "win/windows.graphics.capture.h"
#include "win/windows.graphics.directx.direct3d11.interop.h"
#include "IIDUtils.h"
#include "CallbackImpl.h"
#include "IDirect3DDxgiInterfaceAccess.h"

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

struct SurfaceTranslate {
    GUID GUID_IDirect3DDxgiInterfaceAccess;
    GUID GUID_IDXGISurface;
    int running;
    ID3D11Device *d3d11Device;
    CSizeInt32 framePoolSize;
    CIDirect3DDevice *ciDirect3DDevice;
    int numberOfBuffers;
};;

void onFrameArrive(struct ImplComCallback *This,
                   __x_ABI_CWindows_CGraphics_CCapture_CIDirect3D11CaptureFramePool *framePool,
                   IInspectable *args) {
    CFrame *frame;
    framePool->lpVtbl->TryGetNextFrame(framePool, &frame);
    CSizeInt32 frameSize;
    frame->lpVtbl->get_ContentSize(frame, &frameSize);
    CIDirect3DSurface *surface;
    frame->lpVtbl->get_Surface(frame, &surface);
    CDirect3DSurfaceDescription surfaceDescription;
    surface->lpVtbl->get_Description(surface, &surfaceDescription);
    struct SurfaceTranslate *surfaceTranslate = This->userPtr;
    CIDirect3DDxgiInterfaceAccess *dDxgiInterfaceAccess;
    surface->lpVtbl->QueryInterface(surface, &surfaceTranslate->GUID_IDirect3DDxgiInterfaceAccess,
                                    &dDxgiInterfaceAccess);
    IDXGISurface *idxgiSurface;
    dDxgiInterfaceAccess->lpVtbl->GetInterface(dDxgiInterfaceAccess, &surfaceTranslate->GUID_IDXGISurface,
                                               &idxgiSurface);
    // outer
    fprintf(stderr, "surface{%d,%d}\n", surfaceDescription.Width, surfaceDescription.Height);
    fprintf(stderr, "frame{%d,%d}\n", frameSize.Width, frameSize.Height);

    // outer end
    frame->lpVtbl->Release(frame); // release to be able to get next frame
    if (frameSize.Height != surfaceTranslate->framePoolSize.Height ||
        frameSize.Width != surfaceTranslate->framePoolSize.Width) {
        surfaceTranslate->framePoolSize.Height = frameSize.Height;
        surfaceTranslate->framePoolSize.Width = frameSize.Width;
        framePool->lpVtbl->Recreate(framePool, surfaceTranslate->ciDirect3DDevice,
                                    surfaceDescription.Format, surfaceTranslate->numberOfBuffers,
                                    surfaceTranslate->framePoolSize);
    }
}

int main() {
    CoInitialize(0);
    IID GUID_IGraphicsCaptureItem = iid_utils_guidFrom("79c3f95b-31f7-4ec2-a464-632ef5d30760");
    CIGraphicsCaptureItem *CaptureItem;
    HRESULT ret = graphicsCaptureItemInteropFunc()->lpVtbl->CreateForMonitor(graphicsCaptureItemInteropFunc(), NULL,
                                                                             &GUID_IGraphicsCaptureItem,
                                                                             &CaptureItem);
    CSizeInt32 frameSize;
    CaptureItem->lpVtbl->get_Size(CaptureItem, &frameSize);
    ID3D11Device *d3d11Device;
    ret = D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, D3D11_CREATE_DEVICE_BGRA_SUPPORT,
                            NULL, 0, D3D11_SDK_VERSION, &d3d11Device, NULL, NULL);

    IDXGIDevice *dxgiDevice;
    GUID dxgiUID = iid_utils_guidFrom("54ec77fa-1377-44e6-8c32-88fd5f44c84c");
    ret = d3d11Device->lpVtbl->QueryInterface(d3d11Device, &dxgiUID, &dxgiDevice);
    IInspectable *d3dInspectable;
    CreateDirect3D11DeviceFromDXGIDevice(dxgiDevice, &d3dInspectable);
    GUID winrtD3DDUID = iid_utils_guidFrom("a37624ab-8d5f-4650-9d3e-9eae3d9bc670");
    CIDirect3DDevice *ciDirect3DDevice;
    d3dInspectable->lpVtbl->QueryInterface(d3dInspectable, &winrtD3DDUID, &ciDirect3DDevice);

    struct SurfaceTranslate surfaceTranslate = {
            iid_utils_guidFrom("A9B3D012-3DF2-4EE3-B8D1-8695F457D3C1"),
            iid_utils_guidFrom("cafcb56c-6ac3-4889-bf47-9e23bbd260ec"),
            1, d3d11Device, frameSize, ciDirect3DDevice, 2};

    CIDirect3D11CaptureFramePool *framePool;
    // maybe try CreateFreeThreaded
    ret = direct3D11CaptureFramePoolStaticsFunc()->lpVtbl->Create(direct3D11CaptureFramePoolStaticsFunc(),
                                                                  ciDirect3DDevice,
                                                                  DirectXPixelFormat_B8G8R8A8UIntNormalized,
                                                                  surfaceTranslate.numberOfBuffers,
                                                                  frameSize, &framePool);
    CIGraphicsCaptureSession *captureSession;
    framePool->lpVtbl->CreateCaptureSession(framePool, CaptureItem, &captureSession);
    CDirect3D11CaptureFramePool_IInspectable *fpInspectable = createInspectable(onFrameArrive, &surfaceTranslate);
    EventRegistrationToken token;
    ret = framePool->lpVtbl->add_FrameArrived(framePool, fpInspectable, &token);
    ret = captureSession->lpVtbl->StartCapture(captureSession);
    MSG msg = {};
    while (GetMessageW(&msg, NULL, 0, 0)) {
        if (!surfaceTranslate.running) {
            captureSession->lpVtbl->Release(captureSession);
            framePool->lpVtbl->Release(framePool);
        }
        TranslateMessage(&msg);// onFrameArrive loop thread
        DispatchMessageW(&msg);
    }
}