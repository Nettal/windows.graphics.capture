cmake_minimum_required(VERSION 3.28)
project(capture)

set(CMAKE_CXX_STANDARD 20)

include_directories(.)

add_executable(wgc
        capture/wgc/windows_graphics_capture.c
        capture/wgc/CallbackImpl.c
        capture/wgc/CallbackImpl.h
        capture/wgc/IIDUtils.c
        capture/wgc/IIDUtils.h
        capture/wgc/include/windows.graphics.directx.direct3d11.interop.h
        capture/wgc/IDirect3DDxgiInterfaceAccess.h
        capture/main.cpp
        capture/FrameProcessor.cpp
        capture/FrameProcessor.h
        shared/log_helper.c
        shared/stb_image_write.c
        capture/FrameSender.cpp
        capture/FrameSender.h
        capture/DXGIMapping.cpp
        capture/DXGIMapping.h
        capture/D3D11Context.cpp
        capture/D3D11Context.h
        capture/AbstractCapture.h
        capture/wgc/WGCCapture.cpp
        capture/wgc/WGCCapture.h
        capture/CaptureShared.h
        shared/network.cpp
        capture/AbstractFrameProcessor.h
)

add_executable(display
        display/Display.cpp
        display/Display.h
        shared/log_helper.c
        shared/log_helper.h
        display/glad.c
        display/glad.h
        shared/network.cpp
        shared/shared.h
        display/RenderTarget.cpp
        display/RenderTarget.h
        shared/stb_image_write.c
)

target_link_libraries(display glfw3 ws2_32 lz4)

target_link_libraries(wgc D3D11 d3dcompiler lz4 ws2_32)
