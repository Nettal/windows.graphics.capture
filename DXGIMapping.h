//
// Created by Nettal on 2024/5/26.
//

#ifndef WGC_DXGIMAPPING_H
#define WGC_DXGIMAPPING_H


#include <dxgi.h>
#include <d3d11.h>
#include "windows_graphics_capture.h"

class DXGIMapping {
    IDXGISurface *dxgiSurface{};
    ID3D11Texture2D *cpuAccessingTexture{};
    ID3D11DeviceContext *deviceCtx{};
public:
    D3D11_TEXTURE2D_DESC frameDesc{};
    DXGI_MAPPED_RECT mappedRect{};

    explicit DXGIMapping(ID3D11Device *d3d11Device, WGC_SIZE2D currentTextureSize, ID3D11DeviceContext *deviceCtx);

    explicit DXGIMapping() = default;

    void copy(ID3D11Texture2D *renderTarget);

    void free();
};


#endif //WGC_DXGIMAPPING_H
