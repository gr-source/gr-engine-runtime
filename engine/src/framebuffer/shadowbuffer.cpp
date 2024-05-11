#include "shadowbuffer.h"

#include "../settings.h"

Shadowbuffer &Shadowbuffer::getInstance() {
    static Shadowbuffer m_shadowbuffer(Settings::GameGrapicQiality.x, Settings::GameGrapicQiality.y);
    return m_shadowbuffer;
}

Shadowbuffer::Shadowbuffer(u32 width, u32 height) : m_width(width), m_height(height), m_framebuffer(std::make_unique<Framebuffer>()), m_texture(std::make_unique<Texture2D>()) {
    m_texture->resize(width, height, Texture::eDEPTH_COMPONENT, Texture::eDEPTH_COMPONENT, Texture::ePOINT, Texture::eFLOAT, false, Texture::eCLAMPBORDER);
    m_texture->apply();

    m_framebuffer->bind();
    m_framebuffer->SetTexture2D(Framebuffer::eDEPTH_ATTACHMENT, Framebuffer::eTexture2D, m_texture.get());
    m_framebuffer->none();
    m_framebuffer->unbind();
}

void Shadowbuffer::bind() {
    GSetViewport(m_width, m_height);
    m_framebuffer->bind();
}

void Shadowbuffer::unbind() {
    m_framebuffer->unbind();
}

Texture2D *Shadowbuffer::GetTexture() {
    return m_texture.get();
}
