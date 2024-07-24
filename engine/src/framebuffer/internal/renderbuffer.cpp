#include "renderbuffer.h"

Renderbuffer::Renderbuffer() : m_rbo(-1) {
    GL_CHECK(glGenRenderbuffers(1, &m_rbo));
}

Renderbuffer::~Renderbuffer() {
    if (m_rbo != -1) {
        GL_CHECK(glDeleteRenderbuffers(1, &m_rbo));
    }
}

void Renderbuffer::bind() {
    GL_CHECK(glBindRenderbuffer(GL_RENDERBUFFER, m_rbo));
}

void Renderbuffer::unbind() {
    GL_CHECK(glBindRenderbuffer(GL_RENDERBUFFER, 0));
}

GLuint Renderbuffer::getRBO() {
    return m_rbo;
}

void Renderbuffer::bufferStorage(u32 internalFormat, u32 width, u32 height) {
    GL_CHECK(glRenderbufferStorage(GL_RENDERBUFFER, internalFormat, width, height));
}

void Renderbuffer::bufferStorageSampler(u32 internalFomat, u32 samples, u32 width, u32 height) {
    GL_CHECK(glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, internalFomat, width, height));
}
