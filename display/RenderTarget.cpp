//
// Created by SnowNF on 2024/5/28.
//

#include "RenderTarget.h"
#include "../shared/log_helper.h"

RenderTarget::RenderTarget(int width, int height) : w(width), h(height) {
    std::cout << "new RenderTarget w: " << w << " h: " << h << std::endl;
    //生成帧缓冲区对象
    glGenFramebuffers(1, &m_FBO);
    bind();

    glGenTextures(ATTACHMENT_NUM, static_cast<GLuint *>(m_AttachTexIds));
    for (int i = 0; i < ATTACHMENT_NUM; ++i) {
        glBindTexture(GL_TEXTURE_2D, m_AttachTexIds[i]);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, nullptr);
        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, attachments[i], GL_TEXTURE_2D, m_AttachTexIds[i], 0);
    }
    // now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now
    GLenum stat = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (stat != GL_FRAMEBUFFER_COMPLETE)
        mw_fatal("ERROR::FRAMEBUFFER:: Framebuffer is not complete! %#x", stat);
    glDrawBuffers(ATTACHMENT_NUM, attachments);
    restore();
}

int RenderTarget::getTextureCount() {
    return ATTACHMENT_NUM;
}

GLuint RenderTarget::getTextureId(int index) {
    return m_AttachTexIds[index];
}

void RenderTarget::bind() {
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &defaultFrameBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
}

void RenderTarget::close() const {
    for (int i = 0; i < ATTACHMENT_NUM; ++i) {
        glDeleteTextures(1, &m_AttachTexIds[i]);
    }
    glDeleteFramebuffers(1, &m_FBO);
}

void RenderTarget::restore() const {
    glBindFramebuffer(GL_FRAMEBUFFER, defaultFrameBuffer);
}
