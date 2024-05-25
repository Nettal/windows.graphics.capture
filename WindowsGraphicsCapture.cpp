//
// Created by Nettal on 2024/5/25.
//

#include <iostream>
#include "WindowsGraphicsCapture.h"
#include "stb_image_write.h"

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
        "Texture2D<float4> textureSampler : register(t0); // texture sampler declaration\n"
        "SamplerState textureSamplerSampler\n"
        "{\n"
        "    Filter = MIN_MAG_MIP_POINT;\n"
        "    AddressU = Wrap;\n"
        "    AddressV = Wrap;\n"
        "};"
        "\n"
        "vs_out vs_main(vs_in input) {\n"
        "    vs_out output = (vs_out)0; // zero the memory first\n"
        "    output.position_clip = float4(input.position_local, 1.0);\n"
        "    output.texCoord = input.texCoord; // pass texCoord to pixel shader\n"
        "    return output;\n"
        "}\n"
        "\n"
        "float4 ps_main(vs_out input) : SV_TARGET {\n"
        "    float4 textureColor = textureSampler.Sample(textureSamplerSampler, input.texCoord); // sample the texture\n"
        "    return textureColor;\n"
        "}";
#define CHECK_RESULT(x) do{if(FAILED(x)) {fprintf(stderr,"error at %s:%d",__FILE__, __LINE__);}} while(0)

WindowsGraphicsCapture::WindowsGraphicsCapture() {
    auto d3dDeviceExtensions = {
            D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_10_1,
            D3D_FEATURE_LEVEL_10_0,
            D3D_FEATURE_LEVEL_9_1};
    int d3d_flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
    if (enable_d3d_debug)
        d3d_flags |= D3D11_CREATE_DEVICE_DEBUG;
    D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, d3d_flags,
                      d3dDeviceExtensions.begin(), d3dDeviceExtensions.size(),
                      D3D11_SDK_VERSION, &d3d11Device, nullptr, &d3d11DeviceContext);
    wgc_c_internal = wgc_initial_everything(nullptr, &wgcFrameSIze, d3d11Device, receiveWGCFrame, this);
    vertexShaderBlob = compileShader(hlsl_shader, "vs_main", "vs_5_0");
    fragmentShaderBlob = compileShader(hlsl_shader, "ps_main", "ps_5_0");
    D3D11_TEXTURE2D_DESC frame_desc{};
    frame_desc.Height = wgcFrameSIze.height;
    frame_desc.Width = wgcFrameSIze.width;
    frame_desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    frame_desc.Usage = D3D11_USAGE_DEFAULT;
    frame_desc.BindFlags = D3D11_BIND_RENDER_TARGET;
    frame_desc.MiscFlags = 0;
    frame_desc.MipLevels = 1;
    frame_desc.ArraySize = 1;
    frame_desc.SampleDesc.Count = 1;
    frame_desc.SampleDesc.Quality = 0;
    auto hr = d3d11Device->CreateTexture2D(&frame_desc, nullptr, &renderTargetTexture);
    CHECK_RESULT(hr);
    hr = d3d11Device->CreateRenderTargetView(renderTargetTexture, nullptr, &renderTargetImageView);
    CHECK_RESULT(hr);

    frame_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
    hr = d3d11Device->CreateTexture2D(&frame_desc, nullptr, &frameSamplerTexture);
    CHECK_RESULT(hr);
    frame_desc.Usage = D3D11_USAGE_STAGING;
    frame_desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
    frame_desc.BindFlags = 0;
    hr = d3d11Device->CreateTexture2D(&frame_desc, nullptr, &cpuAccessingTexture);
    CHECK_RESULT(hr);
    hr = d3d11Device->CreateShaderResourceView(frameSamplerTexture, nullptr, &samplerImageView);
    CHECK_RESULT(hr);

    D3D11_BUFFER_DESC vertexInputDescriptor;
    vertexInputDescriptor.ByteWidth = sizeof(deferredVertexInput.size() * sizeof(Vertex));
    vertexInputDescriptor.Usage = D3D11_USAGE_DEFAULT;
    vertexInputDescriptor.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexInputDescriptor.CPUAccessFlags = 0;
    vertexInputDescriptor.MiscFlags = 0;
    vertexInputDescriptor.StructureByteStride = 0;
    D3D11_SUBRESOURCE_DATA vertexInputData;
    vertexInputData.pSysMem = deferredVertexInput.data();
    vertexInputData.SysMemPitch = 0;
    vertexInputData.SysMemSlicePitch = 0;
    hr = d3d11Device->CreateBuffer(&vertexInputDescriptor, &vertexInputData, &deferredVertexBuffer);
    CHECK_RESULT(hr);
    D3D11_INPUT_ELEMENT_DESC vertexInputAttributeDescription[] =
            {
                    {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0},
                    {"TEXCOORD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
            };
    hr = d3d11Device->CreateInputLayout(vertexInputAttributeDescription,
                                        sizeof(vertexInputAttributeDescription) / sizeof(D3D11_INPUT_ELEMENT_DESC),
                                        vertexShaderBlob->GetBufferPointer(),
                                        vertexShaderBlob->GetBufferSize(), &vertexInputLayout);
    CHECK_RESULT(hr);
    hr = d3d11Device->CreateVertexShader(vertexShaderBlob->GetBufferPointer(),
                                         vertexShaderBlob->GetBufferSize(), nullptr, &vertexShader);
    CHECK_RESULT(hr);

    hr = d3d11Device->CreatePixelShader(fragmentShaderBlob->GetBufferPointer(),
                                        fragmentShaderBlob->GetBufferSize(), nullptr, &fragmentShader);
    CHECK_RESULT(hr);
    D3D11_SAMPLER_DESC samplerDesc = {};
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR; // 设置采样器过滤器
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP; // 设置纹理寻址模式
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    d3d11Device->CreateSamplerState(&samplerDesc, &frameSamplerState);
    CHECK_RESULT(hr);
}

void
WindowsGraphicsCapture::receiveWGCFrame(OnFrameArriveParameter *para, OnFrameArriveRet *ret) {
    auto this_ = reinterpret_cast<WindowsGraphicsCapture *>(para->userPtr);
    ret->running = this_->running;
    this_->d3d11DeviceContext->CopyResource(this_->cpuAccessingTexture, para->d3d11Texture2D);
    D3D11_TEXTURE2D_DESC texture2DDesc;
    this_->cpuAccessingTexture->GetDesc(&texture2DDesc);
    IDXGISurface *idxgiSurface;
    this_->cpuAccessingTexture->QueryInterface(__uuidof(IDXGISurface), (void **) &idxgiSurface);
    DXGI_MAPPED_RECT mappedRect;
    auto hr = idxgiSurface->Map(&mappedRect, DXGI_MAP_READ);
    CHECK_RESULT(hr);
    stbi_write_png((std::to_string(para->systemRelativeTime) + "capture.png").data(),
                   texture2DDesc.Width, texture2DDesc.Height,
                   4, mappedRect.pBits, texture2DDesc.Width * 4);
//    assert(0);
}

void WindowsGraphicsCapture::doCapture() {
    running = 1;
    // blocked
    wgc_do_capture_on_this_thread(wgc_c_internal);
}

void WindowsGraphicsCapture::stopCapture() {
    running = 0;
}
