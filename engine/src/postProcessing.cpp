#include "postProcessing.hpp"

#include <gr-render/gFramebuffer.h>
#include <gr-render/gTexture.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <gr-base/core/stb/stb_image_write.h>

PostProcessing *PostProcessing::m_current = nullptr;

PostProcessing::PostProcessing(const iVector2 &size) : m_colorTexture(nullptr), m_depthTexture(nullptr), m_fbo(0), m_viewport(size) {
    m_fbo = grr::gFramebuffer::Create();
    if (!m_fbo) {
        throw std::runtime_error("Falied to create framebuffer.");
    }

    m_colorTexture = grr::gTexture::Create();
    if (!m_colorTexture) {
        throw std::runtime_error("Falied to create Texture.");
    }

    m_depthTexture = grr::gTexture::Create();
    if (!m_depthTexture) {
        throw std::runtime_error("Falied to create renderbufer.");
    }

    grr::gTexture::Bind(m_colorTexture);
    grr::gTexture::UpdateTexture(m_viewport.x, m_viewport.y, grr::gTextureFlags_None, grr::TextureFormat_SRGB, nullptr);

    grr::gTexture::Bind(m_depthTexture);
    grr::gTexture::UpdateTexture(m_viewport.x, m_viewport.y, grr::gTextureFlags_None, grr::TextureFormat_DepthComponent, nullptr);

    // Set framebuffer
    grr::gFramebuffer::Bind(m_fbo);

    grr::gTexture::Bind(m_colorTexture, 0);
    grr::gFramebuffer::SetTexture(grr::gFramebufferFlags_Color_Attachiment0);

    grr::gTexture::Bind(m_depthTexture, 0);
    grr::gFramebuffer::SetTexture(grr::gFramebufferFlags_Depth_Attachiment);

    grr::gFramebuffer::Unbind();
}

PostProcessing::~PostProcessing() {
    if (m_colorTexture) {
        delete m_colorTexture;
    }

    if (m_depthTexture) {
        delete m_depthTexture;
    }

    if (m_fbo) {
        grr::gFramebuffer::Destroy(m_fbo);
    }
}

void PostProcessing::resize(const iVector2 &size) {
    m_viewport = size;

    grr::gTexture::Bind(m_colorTexture); // SRG or RGB ???
    grr::gTexture::UpdateTexture(m_viewport.x, m_viewport.y, grr::gTextureFlags_None, grr::TextureFormat_SRGB, nullptr);
    grr::gTexture::Unbind();

    grr::gTexture::Bind(m_depthTexture); // SRG or RGB ???
    grr::gTexture::UpdateTexture(m_viewport.x, m_viewport.y, grr::gTextureFlags_None, grr::TextureFormat_DepthComponent, nullptr);
    grr::gTexture::Unbind();
}

void PostProcessing::bind() {
    grr::gRender::SetRenderState(grr::GR_VIEWPORT, (void *)&m_viewport);
    grr::gFramebuffer::Bind(m_fbo);

    m_current = this;
}

void PostProcessing::unbind() {
    grr::gFramebuffer::Unbind();

    m_current = nullptr;
}

grr::gTexture *PostProcessing::getTextureColor() {
    return m_colorTexture;
}
