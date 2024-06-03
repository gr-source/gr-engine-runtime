#include "gBuffer.h"

#include "../settings.h"
#include "../screen.h"

GBuffer &GBuffer::getInstance() {
    static GBuffer m_gbuffer(Settings::GameGrapicQiality.x, Settings::GameGrapicQiality.y);
    return m_gbuffer;
}

GBuffer::GBuffer(u32 width, u32 height) : m_width(width), m_height(height), m_framebuffer(std::make_unique<Framebuffer>()), m_renderbuffer(std::make_unique<Renderbuffer>()) {
    SetupGBuffer();
}

void GBuffer::SetupGBuffer() {
    m_textures[Texture_Position] = std::make_unique<Texture>();
    m_textures[Texture_Position]->resize(m_width, m_height, Texture::eRGBA32F, Texture::eRGBA, Texture::ePOINT, Texture::eFLOAT, false);
    m_textures[Texture_Position]->apply();

    m_textures[Texture_Normal] = std::make_unique<Texture>();
    m_textures[Texture_Normal]->resize(m_width, m_height, Texture::eRGBA32F, Texture::eRGBA, Texture::ePOINT, Texture::eFLOAT, false);
    m_textures[Texture_Normal]->apply();

    m_textures[Texture_Albedo] = std::make_unique<Texture>();
    m_textures[Texture_Albedo]->resize(m_width, m_height, Texture::eRGBA, Texture::eRGBA, Texture::ePOINT, Texture::eBYTE, false);
    m_textures[Texture_Albedo]->apply();

    m_textures[Texture_Roughnness] = std::make_unique<Texture>();
    m_textures[Texture_Roughnness]->resize(m_width, m_height, Texture::eRED, Texture::eRG, Texture::ePOINT, Texture::eBYTE, false);
    m_textures[Texture_Roughnness]->apply();

    m_textures[Texture_Ao] = std::make_unique<Texture>();
    m_textures[Texture_Ao]->resize(m_width, m_height, Texture::eRED, Texture::eRG, Texture::ePOINT, Texture::eBYTE, false);
    m_textures[Texture_Ao]->apply();

    m_textures[Texture_Metallic] = std::make_unique<Texture>();
    m_textures[Texture_Metallic]->resize(m_width, m_height, Texture::eRED, Texture::eRG, Texture::ePOINT, Texture::eBYTE, false);
    m_textures[Texture_Metallic]->apply();

    m_renderbuffer->bind();
    m_renderbuffer->bufferStorage(Renderbuffer::eDEPTH_COMPONENT, m_width, m_height);
    m_renderbuffer->unbind();

    m_framebuffer->bind();
    
    m_framebuffer->SetTexture2D(Framebuffer::eCOLOR_ATTACHMENT0, Framebuffer::eTexture2D, m_textures[Texture_Position].get());
    m_framebuffer->SetTexture2D(Framebuffer::eCOLOR_ATTACHMENT1, Framebuffer::eTexture2D, m_textures[Texture_Normal].get());
    m_framebuffer->SetTexture2D(Framebuffer::eCOLOR_ATTACHMENT2, Framebuffer::eTexture2D, m_textures[Texture_Albedo].get());
    m_framebuffer->SetTexture2D(Framebuffer::eCOLOR_ATTACHMENT3, Framebuffer::eTexture2D, m_textures[Texture_Roughnness].get());
    m_framebuffer->SetTexture2D(Framebuffer::eCOLOR_ATTACHMENT4, Framebuffer::eTexture2D, m_textures[Texture_Ao].get());
    m_framebuffer->SetTexture2D(Framebuffer::eCOLOR_ATTACHMENT5, Framebuffer::eTexture2D, m_textures[Texture_Metallic].get());

    m_framebuffer->SetRenderbuffer(Framebuffer::eDEPTH_ATTACHMENT, m_renderbuffer.get());
    unsigned int attachments[6] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4, GL_COLOR_ATTACHMENT5};
    GL_CHECK(glDrawBuffers(6, attachments));
    m_framebuffer->unbind();
}

void GBuffer::bind() {
    GSetViewport(m_width, m_height);
    m_framebuffer->bind();
}

void GBuffer::unbind() {
    m_framebuffer->unbind();
}

Texture *GBuffer::GetTexture(u32 type) {
    return m_textures[type].get();
}
