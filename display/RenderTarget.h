//
// Created by SnowNF on 2024/5/28.
//

#ifndef WGC_RENDERTARGET_H
#define WGC_RENDERTARGET_H

#include "glad.h"
#include <iostream>

class RenderTarget {
    GLuint m_FBO{};
    // 一个用来存放上个pass的内容
#define ATTACHMENT_NUM 2
    GLuint m_AttachTexIds[ATTACHMENT_NUM] = {};
    const GLenum attachments[ATTACHMENT_NUM] = {
            GL_COLOR_ATTACHMENT1,
            GL_COLOR_ATTACHMENT2,
    };
    GLint defaultFrameBuffer = GL_NONE;
    int w;
    int h;
public:
    RenderTarget(int width, int height);

    static int getTextureCount();

    GLuint getTextureId(int index);

    void bind();

    void restore() const;

    void close() const;
};


#endif //WGC_RENDERTARGET_H
