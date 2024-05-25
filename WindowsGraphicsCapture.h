//
// Created by Nettal on 2024/5/25.
//

#ifndef WGC_WINDOWSGRAPHICSCAPTURE_H
#define WGC_WINDOWSGRAPHICSCAPTURE_H


#include <d3d11.h>
#include "windows_graphics_capture.h"

class WindowsGraphicsCapture {
    ID3D11Device *d3d11Device{};
    void *wgc_c_internal{};
    int running{};

    static void receiveWGCFrame(OnFrameArriveParameter *onFrameArriveParameter, OnFrameArriveRet *arriveRet);

public:
    explicit WindowsGraphicsCapture();

    void doCapture();

    void stopCapture();
};


#endif //WGC_WINDOWSGRAPHICSCAPTURE_H
