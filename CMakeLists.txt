cmake_minimum_required(VERSION 3.28)
project(wgc)

set(CMAKE_CXX_STANDARD 20)

include_directories(.)

add_executable(wgc
        wgc/windows_graphics_capture.c
        main.h
        wgc/CallbackImpl.c
        wgc/CallbackImpl.h
        wgc/IIDUtils.c
        wgc/IIDUtils.h
        wgc/include/windows.graphics.directx.direct3d11.interop.h
        wgc/IDirect3DDxgiInterfaceAccess.h
        main.cpp
        main.h
        wgc/WindowsGraphicsCapture.cpp
        wgc/WindowsGraphicsCapture.h
        log_helper.c
        stb_image_write.c
        wgc/FrameSender.cpp
        wgc/FrameSender.h
        wgc/DXGIMapping.cpp
        wgc/DXGIMapping.h
        wgc/D3D11Context.cpp
        wgc/D3D11Context.h
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
