#pragma once

#include "FrameBuffer.h"

class Imguibuffer : public FrameBuffer {
public:
    static Imguibuffer& getInstance();
    
    Imguibuffer(u32 width, u32 height);

    void bind() override;
    void unbind() override;

    void resize(const Vector2& size) override;

    GLuint gettexture() override;

    Framebuffer* getframebuffer();

protected:
    std::unique_ptr<Renderbuffer> m_renderbuffer;
    std::unique_ptr<Framebuffer> m_framebuffer;
    std::unique_ptr<Texture> m_texture;
};

