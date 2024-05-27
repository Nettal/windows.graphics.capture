//
// Created by Nettal on 2024/5/28.
//

#ifndef WGC_D3D11CONTEXT_H
#define WGC_D3D11CONTEXT_H


#include <d3d11.h>
#include "log_helper.h"

class D3D11Context {
    static constexpr bool enableD3DDebug = false;
    ID3D11InfoQueue *debugInfoQueue{};
public:
    ID3D11Device *d3d11Device{};
    ID3D11DeviceContext *deviceCtx{};

    void printDX11infos();

    explicit D3D11Context();
};


#endif //WGC_D3D11CONTEXT_H
