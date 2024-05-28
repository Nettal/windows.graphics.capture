//
// Created by SnowNF on 2024/5/28.
//

#ifndef LEARNCPP11_DISPLAY_H
#define LEARNCPP11_DISPLAY_H


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
public:
    void setSize(int _width, int _height);

    void init(int _width, int _height, int _texW, int _texH);

    void terminate();

    static void framebufferSizeCallback(GLFWwindow *window, int width, int height);

    void uploadTex(void *pixels);
};


#endif //LEARNCPP11_DISPLAY_H

