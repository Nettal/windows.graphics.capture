cmake_minimum_required(VERSION 3.28)
project(parsec)

set(CMAKE_CXX_STANDARD 20)

include_directories(.)

add_executable(parsec
        main.c
        main.h
        win/windows.graphics.capture.h
        CallbackImpl.c
        CallbackImpl.h
        IIDUtils.c
        IIDUtils.h
        win/windows.graphics.directx.direct3d11.interop.h
        IDirect3DDxgiInterfaceAccess.h)

target_link_libraries(parsec D3D11)
