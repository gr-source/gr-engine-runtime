#pragma once

#include <gr-render/gRender.h>
#include <gr-math/gmath.h>

class CubeFramebuffer {
public:
    static CubeFramebuffer *Create(const iVector2 &size);

    static void Shutdown(CubeFramebuffer *target);

    static void Bind(CubeFramebuffer *target);

    static void SetTexture(grr::gFramebufferFlags attachment);

    static void Unbind();

    static grr::gTexture *GetTextureColor(CubeFramebuffer *target);

private:
    static CubeFramebuffer *s_current;

    grr::gTexture *m_texture;

    grm::u32 m_rbo;

    grm::u32 m_fbo;

    iVector2 m_viewport;

    CubeFramebuffer();
};
