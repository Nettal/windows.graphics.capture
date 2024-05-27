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
    int width;
    int height;
public:
    void setSize(int width, int height);

    void init(int width, int height);

    void terminate();

    static void framebufferSizeCallback(GLFWwindow *window, int width, int height);
};


#endif //LEARNCPP11_DISPLAY_H

