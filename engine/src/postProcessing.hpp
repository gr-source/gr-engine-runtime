#pragma once

#include <gr-render/gRender.h>

class PostProcessing {
public:
    static void Initialize();

    static void Shutdown();

    static void Resize(const iVector2 &size);

    static void Bind();

    static void Unbind();

    static grr::gTexture *GetTextureColor();

private:
    static grr::gTexture *m_texture;

    static grr::u32 m_rbo;

    static grr::u32 m_fbo;

    static iVector2 m_viewport;
};
