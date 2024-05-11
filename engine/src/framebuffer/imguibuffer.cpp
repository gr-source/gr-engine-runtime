#include "imguibuffer.h"

#include "../screen.h"

Imguibuffer& Imguibuffer::getInstance() {
    static Imguibuffer pImguibuffer(1024, 1024);
    return pImguibuffer;
}

Imguibuffer::Imguibuffer(u32 width, u32 height) : FrameBuffer(width, height), m_framebuffer(std::make_unique<Framebuffer>()), m_renderbuffer(std::make_unique<Renderbuffer>()) {
    m_texture = std::make_unique<Texture2D>();
    
    m_texture->resize(m_width, m_height, Texture::eSRBG8, Texture::eRGB, Texture::eBILINEAR, Texture::eBYTE, false);
    m_texture->apply();

    // m_renderbuffer->bind();
    // m_renderbuffer->bufferStorage(Renderbuffer::eDEPTH_COMPONENT, width, height);
    // m_renderbuffer->unbind();

    m_framebuffer->bind();
    m_framebuffer->SetTexture2D(Framebuffer::eCOLOR_ATTACHMENT0, Framebuffer::eTexture2D, m_texture.get());
    // m_framebuffer->bufferRender(Framebuffer::eDEPTH_ATTACHMENT, m_renderbuffer.get());
    m_framebuffer->unbind();
}

void Imguibuffer::bind() {
    GSetViewport(m_width, m_height);
    m_framebuffer->bind();
}

void Imguibuffer::unbind() {
    m_framebuffer->unbind();
}

void Imguibuffer::resize(const Vector2 &size) {
    m_width = size.x;
    m_height = size.y;

    m_texture->resize(m_width, m_height, Texture::eSRBG8, Texture::eRGB, Texture::eBILINEAR, Texture::eBYTE, false);
    m_texture->apply();
}

GLuint Imguibuffer::gettexture() {
    return m_texture->getID();
}

Framebuffer *Imguibuffer::getframebuffer() {
    return m_framebuffer.get();
}
