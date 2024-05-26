//
// Created by Nettal on 2024/5/25.
//

#ifndef WGC_WINDOWSGRAPHICSCAPTURE_H
#define WGC_WINDOWSGRAPHICSCAPTURE_H


#include <d3d11.h>
#include <d3dcompiler.h>
#include <vector>
#include "windows_graphics_capture.h"
#include "log_helper.h"
#include "./concurrent/blockingconcurrentqueue.h"

using moodycamel::BlockingConcurrentQueue;

class WindowsGraphicsCapture {
    // wgc
    void *wgc_c_internal{};
    WGC_SIZE2D currentTextureSize{};
    // d3d
    bool enable_d3d_debug{};
    ID3D11Device *d3d11Device{};
    ID3D11DeviceContext *deviceCtx{};
    ID3DBlob *vertexShaderBlob{};
    ID3DBlob *fragmentShaderBlob{};
    ID3D11Buffer *deferredVertexBuffer{};
    ID3D11InputLayout *vertexInputLayout{};
    ID3D11VertexShader *vertexShader{};
    ID3D11PixelShader *fragmentShader{};
    ID3D11ShaderResourceView *samplerImageAView{};
    ID3D11ShaderResourceView *samplerImageBView{};
    ID3D11RenderTargetView *renderTargetImageView{};
    ID3D11Texture2D *renderTargetTexture{};
    ID3D11Texture2D *frameSamplerATexture{};
    ID3D11Texture2D *frameSamplerBTexture{};
    ID3D11SamplerState *frameSamplerState{};
    ID3D11InfoQueue* debugInfoQueue;
    int frameCount = 0;
    uint64_t frameTime = 0;
    struct Vertex {
        float position[3];
        float color[4];
    };
    std::vector<Vertex> deferredVertexInput{
            {{-1.0f, -1.0f, 0.0f}, {0,    1,    0, 0}},
            {{-1.0f, 1.0f,  0.0f}, {0.0f, 0.0f, 0, 0}},
            {{1.0f,  1.0f,  0.0f}, {1.0f, 0.0f, 0, 0}},

            {{-1.0f, -1.0f, 0.0f}, {0,    1,    0, 0}},
            {{1.0f,  1.0f,  0.0f}, {1.0f, 0.0f, 0, 0}},
            {{1.0f,  -1.0f, 0.0f}, {1.0f, 1.0f, 0, 0}}};

    class DXGIMapping {
        IDXGISurface *dxgiSurface{};
        ID3D11Texture2D *cpuAccessingTexture{};
        ID3D11DeviceContext *deviceCtx{};
    public:
        D3D11_TEXTURE2D_DESC frame_desc{};
        DXGI_MAPPED_RECT mappedRect{};

        explicit DXGIMapping(ID3D11Device *d3d11Device, WGC_SIZE2D currentTextureSize, ID3D11DeviceContext *deviceCtx);

        void map(ID3D11Texture2D *renderTarget);

        void unmap();

        void free();
    };

    // state
    BlockingConcurrentQueue<DXGIMapping> mapped{2};
    BlockingConcurrentQueue<DXGIMapping> unmapped{2};
    int running{};
    int preTextureIndex{}; // a is 0, b is 1
    int refresh{};

    static void receiveWGCFrame(OnFrameArriveParameter *para, OnFrameArriveRet *ret);

    void doDiffer(ID3D11ShaderResourceView *newView, ID3D11ShaderResourceView *oldView);

    static ID3DBlob *compileShader(const std::string &shader, const std::string &entrance, const std::string &target);

public:

    explicit WindowsGraphicsCapture();

    void doCapture();

    void stopCapture();

    void printDX11infos();
};


#endif //WGC_WINDOWSGRAPHICSCAPTURE_H
