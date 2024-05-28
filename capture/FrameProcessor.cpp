//
// Created by Nettal on 2024/5/25.
//

#include <iostream>
#include <utility>
#include "FrameProcessor.h"
#include "shared/stb_image_write.h"

static const auto hlsl_shader =
        "/* vertex attributes go here to input to the vertex shader */\n"
        "struct vs_in {\n"
        "    float3 position_local : POSITION;\n"
        "    float4 texCoord : TEXCOORD;\n"
        "};\n"
        "\n"
        "/* outputs from vertex shader go here. can be interpolated to pixel shader */\n"
        "struct vs_out {\n"
        "    float4 position_clip : SV_POSITION; // required output of VS\n"
        "    float4 texCoord : TEXCOORD; // pass texCoord to pixel shader\n"
        "};\n"
        "\n"
        "Texture2D<float4> samplerNew : register(t0); // texture sampler declaration\n"
        "Texture2D<float4> samplerOld : register(t1); // texture sampler declaration\n"
        "SamplerState samplerStateNew;\n"
        "SamplerState samplerStateOld;\n"
        "\n"
        "vs_out vs_main(vs_in input) {\n"
        "    vs_out output = (vs_out)0; // zero the memory first\n"
        "    output.position_clip = float4(input.position_local, 1.0);\n"
        "    output.texCoord = input.texCoord; // pass texCoord to pixel shader\n"
        "    return output;\n"
        "}\n"
        "\n"
        "float4 ps_main(vs_out input) : SV_TARGET {\n"
        "    float4 colorNew = samplerNew.Sample(samplerStateNew, input.texCoord); // sample the texture\n"
        "    float4 colorOld = samplerOld.Sample(samplerStateOld, input.texCoord); // sample the texture\n"
        "    if(any(colorNew != colorOld)){\n"
        "       return colorNew.a == 0 ? float4(1, 1, 1, 0) : colorNew;\n"
        "       }"
        "    return float4(0, 0, 0, 0);"
        "}";
#define CHECK_RESULT(x) do{if(FAILED(x)) {fprintf(stderr,"error at %s:%d\n",__FILE__, __LINE__);}} while(0)

FrameProcessor::FrameProcessor(const std::shared_ptr<D3D11Context> &d3dContext, std::shared_ptr<FrameSender> sender) :
        d3dContext(d3dContext), sender(std::move(sender)) {
    vertexShaderBlob = compileShader(hlsl_shader, "vs_main", "vs_5_0");
    fragmentShaderBlob = compileShader(hlsl_shader, "ps_main", "ps_5_0");
    D3D11_BUFFER_DESC vertexInputDescriptor;
    vertexInputDescriptor.ByteWidth = deferredVertexInput.size() * sizeof(Vertex);
    vertexInputDescriptor.Usage = D3D11_USAGE_DEFAULT;
    vertexInputDescriptor.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexInputDescriptor.CPUAccessFlags = 0;
    vertexInputDescriptor.MiscFlags = 0;
    vertexInputDescriptor.StructureByteStride = 0;
    D3D11_SUBRESOURCE_DATA vertexInputData;
    vertexInputData.pSysMem = deferredVertexInput.data();
    vertexInputData.SysMemPitch = 0;
    vertexInputData.SysMemSlicePitch = 0;
    auto hr = d3dContext->d3d11Device->CreateBuffer(&vertexInputDescriptor, &vertexInputData, &deferredVertexBuffer);
    CHECK_RESULT(hr);
    D3D11_INPUT_ELEMENT_DESC vertexInputAttributeDescription[] =
            {
                    {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0},
                    {"TEXCOORD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
            };
    hr = d3dContext->d3d11Device->CreateInputLayout(vertexInputAttributeDescription,
                                                    sizeof(vertexInputAttributeDescription) /
                                                    sizeof(D3D11_INPUT_ELEMENT_DESC),
                                                    vertexShaderBlob->GetBufferPointer(),
                                                    vertexShaderBlob->GetBufferSize(), &vertexInputLayout);
    CHECK_RESULT(hr);
    hr = d3dContext->d3d11Device->CreateVertexShader(vertexShaderBlob->GetBufferPointer(),
                                                     vertexShaderBlob->GetBufferSize(), nullptr, &vertexShader);
    CHECK_RESULT(hr);

    hr = d3dContext->d3d11Device->CreatePixelShader(fragmentShaderBlob->GetBufferPointer(),
                                                    fragmentShaderBlob->GetBufferSize(), nullptr, &fragmentShader);
    CHECK_RESULT(hr);
    D3D11_SAMPLER_DESC samplerDesc = {};
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR; // 设置采样器过滤器
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP; // 设置纹理寻址模式
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    d3dContext->d3d11Device->CreateSamplerState(&samplerDesc, &frameSamplerState);
    CHECK_RESULT(hr);
    d3dContext->printDX11infos();
}

void FrameProcessor::fitWGCFrame(SIZE2D newSize, enum DXGI_FORMAT format) {
    samplerImageAView->Release();
    samplerImageBView->Release();
    samplerImageZeroView->Release();
    renderTargetImageView->Release();
    renderTargetTexture->Release();
    frameSamplerATexture->Release();
    frameSamplerBTexture->Release();
    frameSamplerZeroTexture->Release();
    createTextures(newSize, format);
}

void FrameProcessor::createTextures(SIZE2D newSize, enum DXGI_FORMAT format) {
    currentTextureSize = newSize;
    D3D11_TEXTURE2D_DESC frame_desc{};
    frame_desc.Height = newSize.height;
    frame_desc.Width = newSize.width;
    frame_desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    frame_desc.Usage = D3D11_USAGE_DEFAULT;
    frame_desc.BindFlags = D3D11_BIND_RENDER_TARGET;
    frame_desc.MiscFlags = 0;
    frame_desc.MipLevels = 1;
    frame_desc.ArraySize = 1;
    frame_desc.SampleDesc.Count = 1;
    frame_desc.SampleDesc.Quality = 0;
    auto hr = d3dContext->d3d11Device->CreateTexture2D(&frame_desc, nullptr, &renderTargetTexture);
    CHECK_RESULT(hr);
    hr = d3dContext->d3d11Device->CreateRenderTargetView(renderTargetTexture, nullptr, &renderTargetImageView);
    CHECK_RESULT(hr);
    currentSamplerFormat = format;
    frame_desc.Format = format;// samplers use specified format
    frame_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
    hr = d3dContext->d3d11Device->CreateTexture2D(&frame_desc, nullptr, &frameSamplerATexture);
    CHECK_RESULT(hr);
    hr = d3dContext->d3d11Device->CreateShaderResourceView(frameSamplerATexture, nullptr, &samplerImageAView);
    CHECK_RESULT(hr);

    frame_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
    hr = d3dContext->d3d11Device->CreateTexture2D(&frame_desc, nullptr, &frameSamplerBTexture);
    CHECK_RESULT(hr);
    hr = d3dContext->d3d11Device->CreateShaderResourceView(frameSamplerBTexture, nullptr, &samplerImageBView);
    CHECK_RESULT(hr);

    frame_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
    hr = d3dContext->d3d11Device->CreateTexture2D(&frame_desc, nullptr, &frameSamplerZeroTexture);
    CHECK_RESULT(hr);
    hr = d3dContext->d3d11Device->CreateShaderResourceView(frameSamplerZeroTexture, nullptr, &samplerImageZeroView);
    CHECK_RESULT(hr);
}

void FrameProcessor::doDiffer(ID3D11ShaderResourceView *newView, ID3D11ShaderResourceView *oldView) {
    float clearColor[4] = {0.0f, 0.0f, 0.0f, 0.0f}; // 清空为黑色
    d3dContext->deviceCtx->ClearRenderTargetView(renderTargetImageView, clearColor);

    D3D11_VIEWPORT viewport = {0.0f, 0.0f,
                               (FLOAT) (currentTextureSize.width), (FLOAT) (currentTextureSize.height),
                               0.0f, 1.0f};
    d3dContext->deviceCtx->RSSetViewports(1, &viewport);
    // 设置渲染目标
    d3dContext->deviceCtx->OMSetRenderTargets(1, &renderTargetImageView, nullptr);

    // 设置顶点缓冲区
    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    d3dContext->deviceCtx->IASetVertexBuffers(0, 1, &deferredVertexBuffer, &stride, &offset);
    d3dContext->deviceCtx->IASetInputLayout(vertexInputLayout);
    d3dContext->deviceCtx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    d3dContext->deviceCtx->VSSetShader(vertexShader, nullptr, 0);
    d3dContext->deviceCtx->PSSetShader(fragmentShader, nullptr, 0);

    // 设置纹理资源
    auto imageView = {newView, oldView};
    d3dContext->deviceCtx->PSSetShaderResources(0, imageView.size(), imageView.begin());
    // 将采样器状态绑定到槽位0
    auto samplers = {frameSamplerState, frameSamplerState};
    d3dContext->deviceCtx->PSSetSamplers(0, samplers.size(), samplers.begin());
    // 执行格式转换
    d3dContext->deviceCtx->Draw(6, 0);
    d3dContext->printDX11infos();
}

void FrameProcessor::receiveFrame(OnFrameArriveParameter *para) {
    frameCount++;
    if (((para->systemRelativeTime - frameTime) / 10'000'000.0) >= 1) {
        mw_info("fps:%f", frameCount / ((para->systemRelativeTime - frameTime) / 10'000'000.0));
        frameCount = 0;
        frameTime = para->systemRelativeTime;
    }
    D3D11_TEXTURE2D_DESC wgcTexDesc{};
    para->d3d11Texture2D->GetDesc(&wgcTexDesc);
    if (currentTextureSize.width != para->frameSize.width
        || currentTextureSize.height != para->frameSize.height
        || wgcTexDesc.Format != currentSamplerFormat) {
        fitWGCFrame(para->frameSize, wgcTexDesc.Format);
    }
    ID3D11ShaderResourceView *newView;
    ID3D11ShaderResourceView *oldView;
    if (preTextureIndex == 1) {
        // write to a
        d3dContext->deviceCtx->CopyResource(frameSamplerATexture, para->d3d11Texture2D);
        preTextureIndex = 0;
        newView = samplerImageAView;
        oldView = samplerImageBView;
    } else {
        d3dContext->deviceCtx->CopyResource(frameSamplerBTexture, para->d3d11Texture2D);
        preTextureIndex = 1;
        newView = samplerImageBView;
        oldView = samplerImageAView;
    }
    if (refreshSignal) {
        refreshSignal = 0;
        oldView = samplerImageZeroView;
    }
    doDiffer(newView, oldView);
    sender->waitRequireSlot([this](DXGIMapping &available) -> DXGIMapping & {
        available.copy(renderTargetTexture);
        return available;
    });
}

ID3DBlob *FrameProcessor::compileShader(const std::string &shader, const std::string &entrance,
                                        const std::string &target) {
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

void FrameProcessor::refresh() {
    refreshSignal = 1;
}

void FrameProcessor::preCapture(AbstractCapture *capture) {
    createTextures(capture->currentFrameSize(), DXGI_FORMAT_B8G8R8A8_UNORM);
    sender->preCapture(capture);
}

void FrameProcessor::endCapture(AbstractCapture *capture) {
    vertexShaderBlob->Release();
    fragmentShaderBlob->Release();
    deferredVertexBuffer->Release();
    vertexInputLayout->Release();
    vertexShader->Release();
    fragmentShader->Release();
    samplerImageAView->Release();
    samplerImageBView->Release();
    samplerImageZeroView->Release();
    renderTargetImageView->Release();
    renderTargetTexture->Release();
    frameSamplerATexture->Release();
    frameSamplerBTexture->Release();
    frameSamplerZeroTexture->Release();
    frameSamplerState->Release();
    sender->endCapture(capture);
}
