cmake_minimum_required(VERSION 3.28)
project(wgc)

set(CMAKE_CXX_STANDARD 20)

include_directories(.)

add_executable(wgc
        windows_graphics_capture.c
        main.h
        win/windows.graphics.capture.h
        CallbackImpl.c
        CallbackImpl.h
        IIDUtils.c
        IIDUtils.h
        win/windows.graphics.directx.direct3d11.interop.h
        IDirect3DDxgiInterfaceAccess.h
        main.cpp
        main.h
        WindowsGraphicsCapture.cpp
        WindowsGraphicsCapture.h)

target_link_libraries(wgc D3D11)
