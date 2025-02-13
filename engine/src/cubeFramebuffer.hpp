#pragma once

#include <gr-render/gRender.h>
#include <gr-math/gmath.hpp>

#include <memory>

class CubeFramebuffer
{
public:
    CubeFramebuffer(const iVector2 &size);
    ~CubeFramebuffer();

    void bind() const;

    void unbind() const;

    void setTextureAttachment(grr::gFramebufferFlags attachment);

    grr::gTexture *getCubemapTexture() const;
    
private:
    std::unique_ptr<grr::gTexture> m_texture;

    grm::u32 m_rbo;

    grm::u32 m_fbo;

    iVector2 m_viewport;

    CubeFramebuffer();
};
