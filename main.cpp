//
// Created by Nettal on 2024/5/25.
//

#include <d3d11.h>
#include <iostream>
#include "main.h"
#include "WindowsGraphicsCapture.h"
#include "FrameSender.h"

int main() {
    auto wgc = WindowsGraphicsCapture();
    wgc.doCapture();
}