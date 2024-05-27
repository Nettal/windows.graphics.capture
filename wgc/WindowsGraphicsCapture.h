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
#include "FrameSender.h"
#include "D3D11Context.h"

class WindowsGraphicsCapture {
    // d3d
    D3D11Context d3dContext{};
    WGC_SIZE2D currentTextureSize{};
    enum DXGI_FORMAT currentSamplerFormat{};
    ID3DBlob *vertexShaderBlob{};
    ID3DBlob *fragmentShaderBlob{};
    ID3D11Buffer *deferredVertexBuffer{};
    ID3D11InputLayout *vertexInputLayout{};
    ID3D11VertexShader *vertexShader{};
    ID3D11PixelShader *fragmentShader{};
    ID3D11ShaderResourceView *samplerImageAView{};
    ID3D11ShaderResourceView *samplerImageBView{};
    ID3D11ShaderResourceView *samplerImageZeroView{};
    ID3D11RenderTargetView *renderTargetImageView{};
    ID3D11Texture2D *renderTargetTexture{};
    ID3D11Texture2D *frameSamplerATexture{};
    ID3D11Texture2D *frameSamplerBTexture{};
    ID3D11Texture2D *frameSamplerZeroTexture{};
    ID3D11SamplerState *frameSamplerState{};
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
    // self
    int running{};
    int preTextureIndex{}; // a is 0, b is 1
    int refreshSignal{};
    FrameSender sender{};

    void doDiffer(ID3D11ShaderResourceView *newView, ID3D11ShaderResourceView *oldView);

    static ID3DBlob *compileShader(const std::string &shader, const std::string &entrance, const std::string &target);

    void fitWGCFrame(WGC_SIZE2D newSize, enum DXGI_FORMAT format);

    void createTextures(WGC_SIZE2D newSize, enum DXGI_FORMAT format);

public:

    using FrameSenderProvider = std::function<FrameSender()>;

    explicit WindowsGraphicsCapture(D3D11Context d3dContext,
                                    const FrameSenderProvider &senderProvider,
                                    WGC_SIZE2D textureSize);

    void doCapture();

    void stopCapture();

    void refresh();

    void receiveWGCFrame(OnFrameArriveParameter *para, OnFrameArriveRet *ret);
};


#endif //WGC_WINDOWSGRAPHICSCAPTURE_H
