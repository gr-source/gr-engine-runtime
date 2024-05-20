#pragma once

#include "FrameBuffer.h"

class Shadowbuffer {
public:
    static Shadowbuffer& getInstance();

    Shadowbuffer(u32 width, u32 height);

    void bind();

    void unbind();

    Texture* GetTexture();

protected:
    std::unique_ptr<Framebuffer> m_framebuffer;
    std::unique_ptr<Texture> m_texture;

    u32 m_width;
    u32 m_height;
};


