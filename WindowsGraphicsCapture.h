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

class WindowsGraphicsCapture {
    // wgc
    void *wgc_c_internal{};
    WGC_SIZE2D wgcFrameSIze{};
    // d3d
    bool enable_d3d_debug{};
    ID3D11Device *d3d11Device{};
    ID3D11DeviceContext *d3d11DeviceContext{};
    ID3DBlob *vertexShaderBlob{};
    ID3DBlob *fragmentShaderBlob{};
    ID3D11Buffer *deferredVertexBuffer{};
    ID3D11InputLayout *vertexInputLayout{};
    ID3D11VertexShader *vertexShader{};
    ID3D11PixelShader *fragmentShader{};
    ID3D11ShaderResourceView *samplerImageView{};
    ID3D11RenderTargetView *renderTargetImageView{};
    D3D11_TEXTURE2D_DESC frame_desc{};
    ID3D11Texture2D *renderTargetTexture{};
    ID3D11Texture2D *frameSamplerTexture{};
    ID3D11Texture2D *cpuAccessingTexture{};
    ID3D11SamplerState *frameSamplerState{};

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
    // state
    int running{};

    static void receiveWGCFrame(OnFrameArriveParameter *para, OnFrameArriveRet *ret);

public:
    explicit WindowsGraphicsCapture();

    void doCapture();

    void stopCapture();

    static ID3DBlob *compileShader(const std::string &shader, const std::string &entrance, const std::string &target) {
        ID3DBlob *pBlob = nullptr;
        ID3DBlob *errorMsg = nullptr;
        int flags = D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG;
        HRESULT hr = D3DCompile(shader.c_str(), shader.size(),
                                nullptr, nullptr, nullptr,
                                entrance.c_str(), target.c_str(),
                                flags, 0, &pBlob, &errorMsg);
        if (FAILED(hr)) {
            mw_fatal("fail to compile shader: %s", errorMsg->GetBufferPointer());
        }
        return pBlob;
    }
};


#endif //WGC_WINDOWSGRAPHICSCAPTURE_H
