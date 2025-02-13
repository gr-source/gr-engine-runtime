#include "cubeFramebuffer.hpp"

#include <gr-render/gFramebuffer.h>
#include <gr-render/gTexture.h>

CubeFramebuffer::CubeFramebuffer(const iVector2 &size) : m_viewport(size), m_fbo(0), m_rbo(0), m_texture(std::make_unique<grr::gTexture>()) {
    m_fbo = grr::gFramebuffer::Create();
    if (!m_fbo) {
        throw std::runtime_error("Falied to create framebuffer.");
    }

    m_rbo = grr::gRenderbuffer::Create();
    if (!m_rbo) {
        throw std::runtime_error("Falied to create renderbufer.");
    }

    for (int i = 0; i < 6; ++i) {
        grm::u32 cubemapFlag = 0;
        switch (i) {
            case 0: cubemapFlag = grr::gTextureFlags_Cubemap_Positive_X; break;
            case 1: cubemapFlag = grr::gTextureFlags_Cubemap_Negative_X; break;
            case 2: cubemapFlag = grr::gTextureFlags_Cubemap_Positive_Y; break;
            case 3: cubemapFlag = grr::gTextureFlags_Cubemap_Negative_Y; break;
            case 4: cubemapFlag = grr::gTextureFlags_Cubemap_Positive_Z; break;
            case 5: cubemapFlag = grr::gTextureFlags_Cubemap_Negative_Z; break;
        }

        m_texture->updateBuffer(m_viewport.x, m_viewport.y, grr::gTextureFlags_Cubemap | grr::gTextureFlags_Clamp_Edge | cubemapFlag, grr::TextureFormat_RGB16F, nullptr);
    }

    grr::gRenderbuffer::Bind(m_rbo);
    grr::gRenderbuffer::SetBufferStorage(grr::RenderbufferType_Depth_Component, m_viewport.x, m_viewport.y);
    grr::gRenderbuffer::Unbind();

    // configure framebuffer
    grr::gFramebuffer::Bind(m_fbo);

    grr::gRenderbuffer::Bind(m_rbo);
    grr::gFramebuffer::SetRenderbuffer(grr::gFramebufferFlags_Depth_Attachiment);

    grr::gFramebuffer::Unbind();
}

CubeFramebuffer::~CubeFramebuffer() {
    if (m_fbo) {
        grr::gFramebuffer::Destroy(m_fbo);
    }
}

void CubeFramebuffer::bind() const {
    grr::gRender::SetRenderState(grr::GR_VIEWPORT, (void *)&m_viewport);
    grr::gFramebuffer::Bind(m_fbo);
}

void CubeFramebuffer::unbind() const {
    grr::gFramebuffer::Unbind();
}

void CubeFramebuffer::setTextureAttachment(grr::gFramebufferFlags attachment) {
    m_texture->bind(0);
    
    grr::gFramebuffer::SetTexture(grr::gFramebufferFlags_Color_Attachiment0, attachment);

    m_texture->unbind();
}

grr::gTexture *CubeFramebuffer::getCubemapTexture() const {
    return &(*m_texture);
}


