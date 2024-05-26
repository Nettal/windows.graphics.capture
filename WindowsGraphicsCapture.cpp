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
    if (enableD3DDebug)
        d3d_flags |= D3D11_CREATE_DEVICE_DEBUG;
    D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, d3d_flags,
                      d3dDeviceExtensions.begin(), d3dDeviceExtensions.size(),
                      D3D11_SDK_VERSION, &d3d11Device, nullptr, &deviceCtx);
    if (enableD3DDebug)
        d3d11Device->QueryInterface(__uuidof(ID3D11InfoQueue), (void **) &debugInfoQueue);
    wgc_c_internal = wgc_initial_everything(nullptr, &currentTextureSize, d3d11Device, receiveWGCFrame, this);
    vertexShaderBlob = compileShader(hlsl_shader, "vs_main", "vs_5_0");
    fragmentShaderBlob = compileShader(hlsl_shader, "ps_main", "ps_5_0");
    D3D11_TEXTURE2D_DESC frame_desc{};
    frame_desc.Height = currentTextureSize.height;
    frame_desc.Width = currentTextureSize.width;
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
    hr = d3d11Device->CreateTexture2D(&frame_desc, nullptr, &frameSamplerATexture);
    CHECK_RESULT(hr);
    hr = d3d11Device->CreateShaderResourceView(frameSamplerATexture, nullptr, &samplerImageAView);
    CHECK_RESULT(hr);

    frame_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
    hr = d3d11Device->CreateTexture2D(&frame_desc, nullptr, &frameSamplerBTexture);
    CHECK_RESULT(hr);
    hr = d3d11Device->CreateShaderResourceView(frameSamplerBTexture, nullptr, &samplerImageBView);
    CHECK_RESULT(hr);

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
    printDX11infos();

    std::vector<DXGIMapping> dxgiMaps{BUFFER_NUM};
    for (auto &item: dxgiMaps) {
        item = DXGIMapping{d3d11Device, currentTextureSize, deviceCtx};
    }
    sender = FrameSender{dxgiMaps};
}

void WindowsGraphicsCapture::doDiffer(ID3D11ShaderResourceView *newView, ID3D11ShaderResourceView *oldView) {
    float clearColor[4] = {0.0f, 0.0f, 0.0f, 0.0f}; // 清空为黑色
    deviceCtx->ClearRenderTargetView(renderTargetImageView, clearColor);

    D3D11_VIEWPORT viewport = {0.0f, 0.0f,
                               (FLOAT) (currentTextureSize.width), (FLOAT) (currentTextureSize.height),
                               0.0f, 1.0f};
    deviceCtx->RSSetViewports(1, &viewport);
    // 设置渲染目标
    deviceCtx->OMSetRenderTargets(1, &renderTargetImageView, nullptr);

    // 设置顶点缓冲区
    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    deviceCtx->IASetVertexBuffers(0, 1, &deferredVertexBuffer, &stride, &offset);
    deviceCtx->IASetInputLayout(vertexInputLayout);
    deviceCtx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    deviceCtx->VSSetShader(vertexShader, nullptr, 0);
    deviceCtx->PSSetShader(fragmentShader, nullptr, 0);

    // 设置纹理资源
    auto imageView = {newView, oldView};
    deviceCtx->PSSetShaderResources(0, imageView.size(), imageView.begin());
    // 将采样器状态绑定到槽位0
    auto samplers = {frameSamplerState, frameSamplerState};
    deviceCtx->PSSetSamplers(0, samplers.size(), samplers.begin());
    // 执行格式转换
    deviceCtx->Draw(6, 0);
    printDX11infos();
}

void WindowsGraphicsCapture::receiveWGCFrame(OnFrameArriveParameter *para, OnFrameArriveRet *ret) {
    auto this_ = reinterpret_cast<WindowsGraphicsCapture *>(para->userPtr);
    ret->running = this_->running;
    this_->frameCount++;
    if (((para->systemRelativeTime - this_->frameTime) / 10'000'000.0) >= 1) {
        mw_info("fps:%f", this_->frameCount / ((para->systemRelativeTime - this_->frameTime) / 10'000'000.0));
        this_->frameCount = 0;
        this_->frameTime = para->systemRelativeTime;
    }
    if (this_->preTextureIndex == 1) {
        // write to a
        this_->deviceCtx->CopyResource(this_->frameSamplerATexture, para->d3d11Texture2D);
        this_->preTextureIndex = 0;
        this_->doDiffer(this_->samplerImageAView, this_->samplerImageBView);
    } else {
        this_->deviceCtx->CopyResource(this_->frameSamplerBTexture, para->d3d11Texture2D);
        this_->preTextureIndex = 1;
        this_->doDiffer(this_->samplerImageBView, this_->samplerImageAView);
    }
    this_->sender.waitRequireSlot([this_](DXGIMapping &available) -> DXGIMapping & {
        available.map(this_->renderTargetTexture);
        return available;
    });
}

void WindowsGraphicsCapture::doCapture() {
    running = 1;
    sender.start();
    // blocked
    wgc_do_capture_on_this_thread(wgc_c_internal);
}

void WindowsGraphicsCapture::stopCapture() {
    running = 0;
    sender.stop();
}

ID3DBlob *WindowsGraphicsCapture::compileShader(const std::string &shader, const std::string &entrance,
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

void WindowsGraphicsCapture::printDX11infos() {
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