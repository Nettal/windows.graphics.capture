//
// Created by SnowNF on 2024/5/28.
//

#ifndef LEARNCPP11_DISPLAY_H
#define LEARNCPP11_DISPLAY_H

#include "RenderTarget.h"
#include <GLFW/glfw3.h>
#include <functional>

typedef struct {
    int sizeOfIndices;
    int *indices;
    int sizeOfVert;
    float *verts;
} MeshData;

#define TEXTURE_DIRECT_RENDERING true

#define ENABLE_PBO false

class Display {
    GLFWwindow *window;
    GLFWwindow *sharedThread;
    int width;
    int height;
    int texW;
    int texH;
    GLuint tex;
    void *texData;
#if ENABLE_PBO
    GLuint pbo;
#endif
#if !TEXTURE_DIRECT_RENDERING
    RenderTarget *renderTarget;
#endif
    std::function<void(Display *)> pixelPtrAvailable = [](Display *) {};
public:
    void setSize(int _width, int _height);

    void run(int _width, int _height, int _texW, int _texH);

    void terminate();

    static void framebufferSizeCallback(GLFWwindow *window, int width, int height);

#if ENABLE_PBO
    uint32_t *getPixelPtr();
    void uploadTex();
#else

    void uploadTex(void *pixel);

    void uploadTex(int x, int y, int w, int h, void *pixel);

#endif

    void setPixelPtrAvailable(std::function<void(Display *)> available);

};


#endif //LEARNCPP11_DISPLAY_H

