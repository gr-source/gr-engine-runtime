#include "postProcessing.hpp"

/*
* Estrutura:
*
* FrameBuffer
*   TextureColor (Armazena as cores)
*   RenderBuffer (Armazedar o depth)
*/

grr::gTexture *PostProcessing::m_texture = nullptr;

grr::u32 PostProcessing::m_rbo = 0;

grr::u32 PostProcessing::m_fbo = 0;

iVector2 PostProcessing::m_viewport = {1, 1};

void PostProcessing::Initialize() {
    m_fbo = grr::gFramebuffer::Create();
    if (!m_fbo) {
        throw std::runtime_error("Falied to create framebuffer.");
    }

    m_rbo = grr::gRenderbuffer::Create();
    if (!m_rbo) {
        throw std::runtime_error("Falied to create renderbufer.");
    }

    m_texture = grr::gTexture::Create(m_viewport.x, m_viewport.y, grr::TextureFormat_SRGB, nullptr);
    if (!m_texture) {
        throw std::runtime_error("Falied to create Texture.");
    }

    grr::gRenderbuffer::Bind(m_rbo);
    grr::gRenderbuffer::SetBufferStorage(grr::RenderbufferType_Depth_Component, m_viewport.x, m_viewport.y);
    grr::gRenderbuffer::Unbind();

    grr::gFramebuffer::Bind(m_fbo);
    grr::gFramebuffer::SetTexture(grr::FramebufferType_Color_Attachiment0, m_texture->getID());
    grr::gFramebuffer::SetRenderbuffer(grr::FramebufferType_Depth_Attachiment, m_rbo);
    grr::gFramebuffer::Unbind();
}

void PostProcessing::Shutdown() {
    if (m_texture) {
        delete m_texture;
    }

    if (m_fbo) {
        grr::gFramebuffer::Destroy(m_fbo);
    }
}

void PostProcessing::Resize(const iVector2 &size) {
    if (!m_texture) {
        throw std::runtime_error("Texture invalid.");
    }

    m_viewport = size;

    grr::gTexture::Bind(m_texture, 0); // SRG or RGB ???
    grr::gTexture::PushTexture(m_texture, m_viewport.x, m_viewport.y, grr::TextureFormat_SRGB, nullptr);
    grr::gTexture::Unbind();

    grr::gRenderbuffer::Bind(m_rbo);
    grr::gRenderbuffer::SetBufferStorage(grr::RenderbufferType_Depth_Component, m_viewport.x, m_viewport.y);
    grr::gRenderbuffer::Unbind();
}

void PostProcessing::Bind() {
    grr::gRender::SetRenderState(grr::GR_VIEWPORT, (void *)&m_viewport);
    grr::gFramebuffer::Bind(m_fbo);
}

void PostProcessing::Unbind() {
    grr::gFramebuffer::Unbind();
}

grr::gTexture *PostProcessing::GetTextureColor() {
    return m_texture;
}
