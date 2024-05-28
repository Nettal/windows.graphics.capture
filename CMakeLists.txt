cmake_minimum_required(VERSION 3.28)
project(capture)

set(CMAKE_CXX_STANDARD 20)

include_directories(.)

add_executable(wgc
        capture/wgc/windows_graphics_capture.c
        main.h
        capture/wgc/CallbackImpl.c
        capture/wgc/CallbackImpl.h
        capture/wgc/IIDUtils.c
        capture/wgc/IIDUtils.h
        capture/wgc/include/windows.graphics.directx.direct3d11.interop.h
        capture/wgc/IDirect3DDxgiInterfaceAccess.h
        main.cpp
        main.h
        capture/WindowsGraphicsCapture.cpp
        capture/WindowsGraphicsCapture.h
        log_helper.c
        stb_image_write.c
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
)

add_executable(display
        display/Display.cpp
        display/Display.h
        log_helper.c
        log_helper.h
        display/glad.c
        display/glad.h
)

target_link_libraries(display glfw3)

target_link_libraries(wgc D3D11 d3dcompiler lz4)
