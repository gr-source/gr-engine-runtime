#include "framebuffer.h"

#include "../../resources/texture2D.h"

Framebuffer::Framebuffer() : m_fbo(-1) {
    GL_CHECK(glGenFramebuffers(1, &m_fbo));
}

Framebuffer::~Framebuffer() {
    if (m_fbo != -1) {
        GL_CHECK(glDeleteFramebuffers(1, &m_fbo));
    }
}

void Framebuffer::bind() {
    GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, m_fbo));
}

void Framebuffer::unbind() {
    GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

void Framebuffer::read() {
    GL_CHECK(glBindFramebuffer(GL_READ_FRAMEBUFFER, m_fbo));
}

void Framebuffer::draw() {
    GL_CHECK(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo));
}

void Framebuffer::none() {
    GL_CHECK(glDrawBuffer(GL_NONE));
    GL_CHECK(glReadBuffer(GL_NONE));
}

void Framebuffer::SetTexture2D(u32 attachment, u32 textTarget, Texture *texture) {
    GL_CHECK(glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, textTarget, texture->getID(), 0));
}

void Framebuffer::SetRenderbuffer(u32 attachment, Renderbuffer *renderbuffer) {
    GL_CHECK(glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment, GL_RENDERBUFFER, renderbuffer->getRBO()));
}

const u32& Framebuffer::getID() const {
    return m_fbo;
}
