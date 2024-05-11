#pragma once

#include "FrameBuffer.h"

#include "../resources/textureSample.h"

#define FRAMETEXTURE 0
#define POSTTEXTURE 1

#define FRAMEFBO 0
#define POSFBO 1

class ScreenBuffer {
public:
    static ScreenBuffer& getInstance();
    
    ScreenBuffer(u32 width, u32 height);
    ~ScreenBuffer();

    void bind();
    void unbind();
    void swap();

    void resize(const Vector2& scale);

    void setsample(u32 sample);

    const u32& getsample() const;

    Texture2D* GetTexture();

private:
    std::array<std::unique_ptr<Framebuffer>, 2> m_fbo;
    
    std::unique_ptr<TextureSample> m_textureSample;
    
    std::unique_ptr<Renderbuffer> m_renderbuffer;

    std::unique_ptr<Texture2D> m_texture;

    u32 samples;

    u32 m_width;

    u32 m_height;
};




