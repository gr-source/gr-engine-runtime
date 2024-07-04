#pragma once

#include "../common.h"

#include <gr-render/gRender.h>

class Framebuffer {
public:
    
    static Framebuffer* Create(u32 width, u32 height);

    grr::gTexture* getTexture();

    void bind();

    void updateViewport(u32 width, u32 height);

    void unbind();

    void destroy();

private:
    u32 m_fbo;

    u32 m_rbo;

    u32 m_width;

    u32 m_height;

    grr::gTexture* m_texture;

};




