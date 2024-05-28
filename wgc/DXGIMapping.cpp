//
// Created by Nettal on 2024/5/26.
//

#include "DXGIMapping.h"
#include <cstdio>

#define CHECK_RESULT(x) do{if(FAILED(x)) {fprintf(stderr,"error at %s:%d",__FILE__, __LINE__);}} while(0)

DXGIMapping::DXGIMapping(ID3D11Device *d3d11Device, SIZE2D currentTextureSize,
                         ID3D11DeviceContext *deviceCtx) : deviceCtx(deviceCtx), d3d11Device(d3d11Device) {
    createTexture(currentTextureSize.width, currentTextureSize.height, DXGI_FORMAT_B8G8R8A8_UNORM);
}

void DXGIMapping::createTexture(uint32_t width, uint32_t height, enum DXGI_FORMAT format) {
    frameDesc.Height = height;
    frameDesc.Width = width;
    frameDesc.Format = format;
    frameDesc.Usage = D3D11_USAGE_STAGING;
    frameDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
    frameDesc.BindFlags = 0;
    frameDesc.MiscFlags = 0;
    frameDesc.MipLevels = 1;
    frameDesc.ArraySize = 1;
    frameDesc.SampleDesc.Count = 1;
    frameDesc.SampleDesc.Quality = 0;
    auto hr = d3d11Device->CreateTexture2D(&frameDesc, nullptr, &cpuAccessingTexture);
    CHECK_RESULT(hr);
    hr = cpuAccessingTexture->QueryInterface(__uuidof(IDXGISurface), (void **) &dxgiSurface);
    CHECK_RESULT(hr);
    hr = dxgiSurface->Map(&mappedRect, DXGI_MAP_READ);
    CHECK_RESULT(hr);
}

void DXGIMapping::close() {
    dxgiSurface->Unmap();
    dxgiSurface->Release();
    cpuAccessingTexture->Release();
}

void DXGIMapping::copy(ID3D11Texture2D *renderTarget) {
    D3D11_TEXTURE2D_DESC frameDescTarget{};
    renderTarget->GetDesc(&frameDescTarget);
    if (frameDescTarget.Height != frameDesc.Height ||
        frameDescTarget.Width != frameDesc.Width ||
        frameDescTarget.Format != frameDesc.Format) {
        close();
        createTexture(frameDescTarget.Width, frameDescTarget.Height, frameDescTarget.Format);
    }
    deviceCtx->CopyResource(cpuAccessingTexture, renderTarget);
}