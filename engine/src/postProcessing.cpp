#include "postProcessing.hpp"

#include <gr-render/gFramebuffer.h>
#include <gr-render/gTexture.h>
#include <gr-render/gl.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <gr-base/core/stb/stb_image_write.h>

PostProcessing *PostProcessing::m_current = nullptr;

PostProcessing::PostProcessing(const iVector2 &size) : m_depthTexture(nullptr), m_fbo(0), m_viewport(size) {
    m_colorTexture[0] = nullptr;
    m_colorTexture[1] = nullptr;

    m_fbo = grr::gFramebuffer::Create();
    if (!m_fbo) {
        throw std::runtime_error("Falied to create framebuffer.");
    }

    m_colorTexture[0] = new grr::gTexture();
    if (!m_colorTexture[0]) {
        throw std::runtime_error("Falied to create Texture.");
    }

    m_pickTexture = new grr::gTexture();
    if (!m_pickTexture) {
        throw std::runtime_error("Falied to create Texture.");
    }

    m_depthTexture = new grr::gTexture();
    if (!m_depthTexture) {
        throw std::runtime_error("Falied to create renderbufer.");
    }

    // grr::gTexture::Bind(m_colorTexture[0]);
    m_colorTexture[0]->updateBuffer(m_viewport.x, m_viewport.y, grr::gTextureFlags_None, grr::TextureFormat_RGB, nullptr);

    // grr::gTexture::Bind(m_pickTexture);
    m_pickTexture->updateBuffer(m_viewport.x, m_viewport.y, grr::gTextureFlags_None, grr::TextureFormat_RED_INTEGER, nullptr);

    // grr::gTexture::Bind(m_depthTexture);
    m_depthTexture->updateBuffer(m_viewport.x, m_viewport.y, grr::gTextureFlags_None, grr::TextureFormat_DepthComponent, nullptr);

    // Set framebuffer
    grr::gFramebuffer::Bind(m_fbo);

    m_colorTexture[0]->bind(0);
    grr::gFramebuffer::SetTexture(grr::gFramebufferFlags_Color_Attachiment0);

    m_pickTexture->bind(0);
    grr::gFramebuffer::SetTexture(grr::gFramebufferFlags_Color_Attachiment1);

    m_depthTexture->bind(0);
    grr::gFramebuffer::SetTexture(grr::gFramebufferFlags_Depth_Attachiment);

    GLenum buffers[4] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3};
    glDrawBuffers(4, buffers);

    grr::gFramebuffer::Unbind();
}

PostProcessing::~PostProcessing() {
    if (m_colorTexture[0]) {
        delete m_colorTexture[0];
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

    m_colorTexture[0]->updateBuffer(m_viewport.x, m_viewport.y, grr::gTextureFlags_None, grr::TextureFormat_RGB, nullptr);
    if (m_colorTexture[1] != nullptr) {
        m_colorTexture[1]->updateBuffer(m_viewport.x, m_viewport.y, grr::gTextureFlags_None, grr::TextureFormat_RGB, nullptr);
    }

    m_pickTexture->updateBuffer(m_viewport.x, m_viewport.y, grr::gTextureFlags_None, grr::TextureFormat_RED_INTEGER, nullptr);

    m_depthTexture->updateBuffer(m_viewport.x, m_viewport.y, grr::gTextureFlags_None, grr::TextureFormat_DepthComponent, nullptr);
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

void PostProcessing::setTextureColor(grr::gTexture *texture) {
    grr::gFramebuffer::Bind(m_fbo);

    if (!texture) {
        m_colorTexture[1] = nullptr;
        m_colorTexture[0]->bind(0);
    } else {
        m_colorTexture[1] = texture;
        m_colorTexture[1]->bind(0);
    }
    
    grr::gFramebuffer::SetTexture(grr::gFramebufferFlags_Color_Attachiment0);

    m_pickTexture->bind(0);
    grr::gFramebuffer::SetTexture(grr::gFramebufferFlags_Color_Attachiment1);

    m_depthTexture->bind(0);
    grr::gFramebuffer::SetTexture(grr::gFramebufferFlags_Depth_Attachiment);

    GLenum buffers[4] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3};
    glDrawBuffers(4, buffers);

    grr::gFramebuffer::Unbind();

    resize(m_viewport);
}

grr::gTexture *PostProcessing::getTextureColor() {
    return m_colorTexture[0];
}

void PostProcessing::readPixel(int x, int y, void *data) {
    bind();

    glReadBuffer(GL_COLOR_ATTACHMENT1);

    glReadPixels(x, y, 1, 1, GL_RED_INTEGER, GL_INT, data);

    unbind();
}
