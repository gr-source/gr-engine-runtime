#include "cubeFramebuffer.hpp"

CubeFramebuffer *CubeFramebuffer::s_current = nullptr;

CubeFramebuffer *CubeFramebuffer::Create(const iVector2 &size) {
    auto handle = new CubeFramebuffer();
    handle->m_viewport = size;

    handle->m_fbo = grr::gFramebuffer::Create();
    if (!handle->m_fbo) {
        throw std::runtime_error("Falied to create framebuffer.");
    }

    handle->m_rbo = grr::gRenderbuffer::Create();
    if (!handle->m_rbo) {
        throw std::runtime_error("Falied to create renderbufer.");
    }

    handle->m_texture = grr::gTexture::Create();
    if (!handle->m_texture) {
        throw std::runtime_error("Falied to create Texture.");
    }

    grr::gTexture::Bind(handle->m_texture);
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

        grr::gTexture::UpdateTexture(handle->m_viewport.x, handle->m_viewport.y, grr::gTextureFlags_Cubemap | grr::gTextureFlags_Clamp_Edge | cubemapFlag, grr::TextureFormat_SRGB, nullptr);
    }

    grr::gRenderbuffer::Bind(handle->m_rbo);
    grr::gRenderbuffer::SetBufferStorage(grr::RenderbufferType_Depth_Component, handle->m_viewport.x, handle->m_viewport.y);
    grr::gRenderbuffer::Unbind();

    // configure framebuffer
    grr::gFramebuffer::Bind(handle->m_fbo);

    grr::gRenderbuffer::Bind(handle->m_rbo);
    grr::gFramebuffer::SetRenderbuffer(grr::gFramebufferFlags_Depth_Attachiment);

    grr::gFramebuffer::Unbind();

    return handle;
}

void CubeFramebuffer::Shutdown(CubeFramebuffer *target) {
    if (!target) {
        return;
    }

    if (target->m_texture) {
        delete target->m_texture;
    }

    if (target->m_fbo) {
        grr::gFramebuffer::Destroy(target->m_fbo);
    }

    delete target;
}

void CubeFramebuffer::Bind(CubeFramebuffer *target) {
    assert(target != nullptr);

    grr::gRender::SetRenderState(grr::GR_VIEWPORT, (void *)&target->m_viewport);
    grr::gFramebuffer::Bind(target->m_fbo);

    grr::gTexture::Bind(target->m_texture, 0);

    s_current = target;
}

void CubeFramebuffer::SetTexture(grr::gFramebufferFlags attachment) {
    auto texture = s_current->m_texture;
    assert(texture != nullptr);
    
    grr::gTexture::Bind(texture, 1);
    grr::gFramebuffer::SetTexture(grr::gFramebufferFlags_Color_Attachiment0, attachment);
}

void CubeFramebuffer::Unbind() {
    grr::gFramebuffer::Unbind();
}

grr::gTexture *CubeFramebuffer::GetTextureColor(CubeFramebuffer *target) {
    assert(target != nullptr);
    
    return target->m_texture;
}

CubeFramebuffer::CubeFramebuffer() : m_texture(nullptr), m_rbo(0), m_fbo(0), m_viewport(0, 0) {}


