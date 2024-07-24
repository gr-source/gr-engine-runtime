#include "skyboxbuffer.h"

#include "../screen.h"

Skyboxbuffer::Skyboxbuffer(int width, int height) : FrameBuffer(width, height), m_renderbuffer(std::make_unique<Renderbuffer>()), m_framebuffer(std::make_unique<Framebuffer>()) {
    m_colorSkybox = CreateCubemap(512, 512);

    m_irradiance = CreateCubemap(32, 32);

    m_renderbuffer->bind();
    m_renderbuffer->bufferStorage(Renderbuffer::eDEPTH_COMPONENT24, width, height);
    m_renderbuffer->unbind();

    m_framebuffer->bind();
    m_framebuffer->SetRenderbuffer(Framebuffer::eDEPTH_ATTACHMENT, m_renderbuffer.get());
    m_framebuffer->unbind();

    state = FrameBufferStatus::SUCESS;
}

void Skyboxbuffer::bind() {
    GSetViewport(m_width, m_height);
    m_framebuffer->bind();
}

void Skyboxbuffer::unbind() {
    m_framebuffer->unbind();
}

void Skyboxbuffer::addSkyColor(u32 target, u32 attachment) {
    GL_CHECK(glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, target, m_colorSkybox->getID(), 0));
}

Cubemap *Skyboxbuffer::getSkyColor() {
    return m_colorSkybox.get();
}

void Skyboxbuffer::addIrradiance(u32 target, u32 attachment) {
    GL_CHECK(glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, target, m_irradiance->getID(), 0));
}

Cubemap *Skyboxbuffer::getIrradiance() {
    return m_irradiance.get();
}

std::unique_ptr<Cubemap> Skyboxbuffer::CreateCubemap(u32 width, u32 height) {
    auto data = std::make_unique<Cubemap>(width, height, Texture::eRGB16F, Texture::eRGB, Texture::eTRILINEAR, Texture::eFLOAT, false);
    data->bind();
    for (u32 i=0;i<6;i++) {
        data->apply(Cubemap::ePOSITIVE_X + i, nullptr);
    }
    data->unbind();
    return data;
}
