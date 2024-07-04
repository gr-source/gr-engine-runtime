#include "framebuffer.hpp"

#include <gr-render/gRenderbuffer.h>
#include <gr-render/gFramebuffer.h>
#include <gr-render/gTexture.h>
#include <gr-render/gRender.h>

Framebuffer *Framebuffer::Create(u32 width, u32 height) {
    Framebuffer* framebuffer = new Framebuffer();
    framebuffer->m_fbo = grr::gFramebuffer::Create();
    framebuffer->m_rbo = grr::gRenderbuffer::Create();
    framebuffer->m_texture = grr::gTexture::Create(width, height, grr::TextureFormat_RGB, nullptr);

    grr::gRenderbuffer::Bind(framebuffer->m_rbo);
    grr::gRenderbuffer::SetBufferStorage(grr::RenderbufferType_Depth_Component, width, height);
    grr::gRenderbuffer::Unbind();

    grr::gFramebuffer::Bind(framebuffer->m_fbo);
    grr::gFramebuffer::SetRenderbuffer(grr::FramebufferType_Depth_Attachiment, framebuffer->m_rbo);

    grr::gFramebuffer::SetTexture(grr::FramebufferType_Color_Attachiment0, framebuffer->m_texture->getID());
    grr::gFramebuffer::Unbind();

    return framebuffer;
}

void Framebuffer::bind(u32 width, u32 height) {
    Vector2 scal = {static_cast<float>(width), static_cast<float>(height)};
    grr::gRender::SetRenderState(grr::GR_VIEWPORT, (void*)&scal);

    grr::gFramebuffer::Bind(m_fbo);
}

grr::gTexture *Framebuffer::getTexture() {
    return m_texture;
}

void Framebuffer::unbind() {
    grr::gFramebuffer::Unbind();
}

void Framebuffer::destroy() {
    if (m_fbo) {
        grr::gFramebuffer::Destroy(m_fbo);
    }

    if (m_rbo) {
        grr::gRenderbuffer::Destoy(m_rbo);
    }

    if (m_texture) {
        m_texture->destroy();
    }
    delete this;
}
