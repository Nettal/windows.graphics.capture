//
// Created by Nettal on 2024/5/28.
//

#include "D3D11Context.h"
#include <initializer_list>

D3D11Context::D3D11Context() {
    auto d3dDeviceExtensions = {
            D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_10_1,
            D3D_FEATURE_LEVEL_10_0,
            D3D_FEATURE_LEVEL_9_1};
    int d3d_flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
    if (enableD3DDebug)
        d3d_flags |= D3D11_CREATE_DEVICE_DEBUG;
    D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, d3d_flags,
                      d3dDeviceExtensions.begin(), d3dDeviceExtensions.size(),
                      D3D11_SDK_VERSION, &d3d11Device, nullptr, &deviceCtx);
    if (enableD3DDebug)
        d3d11Device->QueryInterface(__uuidof(ID3D11InfoQueue), (void **) &debugInfoQueue);
}

void D3D11Context::printDX11infos() {
    if (!enableD3DDebug)
        return;
    UINT64 message_count = debugInfoQueue->GetNumStoredMessages();
    for (UINT64 i = 0; i < message_count; i++) {
        SIZE_T message_size = 0;
        debugInfoQueue->GetMessage(i, nullptr, &message_size); //get the size of the message

        D3D11_MESSAGE *message = (D3D11_MESSAGE *) malloc(message_size); //allocate enough space
        debugInfoQueue->GetMessage(i, message, &message_size); //get the actual message

        //do whatever you want to do with it
        mw_debug("Directx11: %.*s", message->DescriptionByteLength, message->pDescription);
        free(message);
    }
    debugInfoQueue->ClearStoredMessages();
}
