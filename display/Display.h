//
// Created by SnowNF on 2024/5/28.
//

#ifndef LEARNCPP11_DISPLAY_H
#define LEARNCPP11_DISPLAY_H

#include "RenderTarget.h"
#include <GLFW/glfw3.h>

typedef struct {
    int sizeOfIndices;
    int *indices;
    int sizeOfVert;
    float *verts;
} MeshData;

class Display {
    GLFWwindow *window;
    GLFWwindow *sharedThread;
    int width;
    int height;
    int texW;
    int texH;
    GLuint tex;
    void *texData;
    GLuint pbo;
    RenderTarget *renderTarget;
    std::function<void(Display *)> pixelPtrAvailable = [](Display *) {};
public:
    void setSize(int _width, int _height);

    void run(int _width, int _height, int _texW, int _texH);

    void terminate();

    static void framebufferSizeCallback(GLFWwindow *window, int width, int height);

    uint32_t *getPixelPtr();

    void setPixelPtrAvailable(std::function<void(Display *)> available);

    void uploadTex();
};


#endif //LEARNCPP11_DISPLAY_H

