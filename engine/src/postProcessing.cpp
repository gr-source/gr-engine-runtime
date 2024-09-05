#include "postProcessing.hpp"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "core/stb/stb_image_write.h"

PostProcessing *PostProcessing::m_current = nullptr;

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

    handle->m_texture = grr::gTexture::Create();
    if (!handle->m_texture) {
        throw std::runtime_error("Falied to create Texture.");
    }

    grr::gTexture::Bind(handle->m_texture);
    grr::gTexture::UpdateTexture(handle->m_viewport.x, handle->m_viewport.y, grr::gTextureFlags_None, grr::TextureFormat_SRGB, nullptr);

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

    grr::gTexture::Bind(target->m_texture); // SRG or RGB ???
    grr::gTexture::UpdateTexture(target->m_viewport.x, target->m_viewport.y, grr::gTextureFlags_None, grr::TextureFormat_SRGB, nullptr);
    grr::gTexture::Unbind();

    grr::gRenderbuffer::Bind(target->m_rbo);
    grr::gRenderbuffer::SetBufferStorage(grr::RenderbufferType_Depth_Component, target->m_viewport.x, target->m_viewport.y);
    grr::gRenderbuffer::Unbind();
}

void PostProcessing::Bind(PostProcessing *target) {
    assert(target != nullptr);

    grr::gRender::SetRenderState(grr::GR_VIEWPORT, (void *)&target->m_viewport);
    grr::gFramebuffer::Bind(target->m_fbo);

    m_current = target;
}

void PostProcessing::Unbind() {
    grr::gFramebuffer::Unbind();

    m_current = nullptr;
}

grr::gTexture *PostProcessing::GetTextureColor(PostProcessing *target) {
    assert(target != nullptr);
    
    return target->m_texture;
}

void PostProcessing::Save(const std::string &path, PostProcessing *postProcessing) {
    if (!postProcessing) {
        throw std::runtime_error("PostProcess pointer is invalid.");
    }

    auto size = postProcessing->m_viewport;

    char pixels[size.x][size.y][3];

    PostProcessing::Bind(postProcessing);
    grr::gFramebuffer::GetPixels(0, 0, size.x, size.y, grr::TextureFormat_RGB, pixels);

    stbi_flip_vertically_on_write(true);
    stbi_write_jpg(path.c_str(), size.x, size.y, 3, pixels, 100);
}

const iVector2 &PostProcessing::GetSize() {
    if (!m_current) {
        throw std::runtime_error("Do not exists post process.");
    }
    return m_current->m_viewport;
}

PostProcessing::PostProcessing() : m_texture(nullptr), m_rbo(0), m_fbo(0), m_viewport(0, 0) {}
