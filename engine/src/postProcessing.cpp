#include "postProcessing.hpp"

/*
* Estrutura:
*
* FrameBuffer
*   TextureColor (Armazena as cores)
*   RenderBuffer (Armazedar o depth)
*/

PostProcessing *PostProcessing::Create(const iVector2 &size) {
    auto handle = new PostProcessing();
    handle->m_viewport = size;

    handle->m_fbo = grr::gFramebuffer::Create();
    if (!handle->m_fbo) {
        throw std::runtime_error("Falied to create framebuffer.");
    }

    handle->m_rbo = grr::gRenderbuffer::Create();
    if (!handle->m_rbo) {
        throw std::runtime_error("Falied to create renderbufer.");
    }

    handle->m_texture = grr::gTexture::Create(handle->m_viewport.x, handle->m_viewport.y, grr::gTextureFlags_None, grr::TextureFormat_SRGB, nullptr);
    if (!handle->m_texture) {
        throw std::runtime_error("Falied to create Texture.");
    }

    grr::gRenderbuffer::Bind(handle->m_rbo);
    grr::gRenderbuffer::SetBufferStorage(grr::RenderbufferType_Depth_Component, handle->m_viewport.x, handle->m_viewport.y);
    grr::gRenderbuffer::Unbind();

    // Set framebuffer
    grr::gFramebuffer::Bind(handle->m_fbo);

    grr::gTexture::Bind(handle->m_texture, 0);
    grr::gFramebuffer::SetTexture(grr::gFramebufferFlags_Color_Attachiment0);

    grr::gRenderbuffer::Bind(handle->m_rbo);
    grr::gFramebuffer::SetRenderbuffer(grr::gFramebufferFlags_Depth_Attachiment);
    grr::gFramebuffer::Unbind();

    return handle;
}

void PostProcessing::Shutdown(PostProcessing *target) {
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

void PostProcessing::Resize(PostProcessing *target, const iVector2 &size) { 
    assert(target != nullptr || target->m_texture != nullptr);

    target->m_viewport = size;

    grr::gTexture::Bind(target->m_texture, 0); // SRG or RGB ???
    grr::gTexture::UpdateTexture(target->m_texture, target->m_viewport.x, target->m_viewport.y, grr::gTextureFlags_None, grr::TextureFormat_SRGB, nullptr);
    grr::gTexture::Unbind();

    grr::gRenderbuffer::Bind(target->m_rbo);
    grr::gRenderbuffer::SetBufferStorage(grr::RenderbufferType_Depth_Component, target->m_viewport.x, target->m_viewport.y);
    grr::gRenderbuffer::Unbind();
}

void PostProcessing::Bind(PostProcessing *target) {
    assert(target != nullptr);

    grr::gRender::SetRenderState(grr::GR_VIEWPORT, (void *)&target->m_viewport);
    grr::gFramebuffer::Bind(target->m_fbo);
}

void PostProcessing::Unbind() {
    grr::gFramebuffer::Unbind();
}

grr::gTexture *PostProcessing::GetTextureColor(PostProcessing *target) {
    assert(target != nullptr);
    
    return target->m_texture;
}

PostProcessing::PostProcessing() : m_texture(nullptr), m_rbo(0), m_fbo(0), m_viewport(0, 0) {}
