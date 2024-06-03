#pragma once

#include <memory>
#include <array>

#include "../types.h"
#include "../common.h"

#include "../resources/texture.h"

#include "internal/framebuffer.h"

#include "internal/renderbuffer.h"

class GBuffer {
public:
    enum {
        Texture_Position = 0,
        Texture_Normal = 1,
        Texture_Albedo = 2,
        Texture_Roughnness = 3,
        Texture_Ao = 4,
        Texture_Metallic = 5,
    };

    static GBuffer& getInstance();

    GBuffer(u32 width, u32 height);

    void bind();

    void unbind();

    Texture* GetTexture(u32 type);
    
    std::unique_ptr<Texture> m_texture;
protected:
    std::array<std::unique_ptr<Texture>, 6> m_textures;

    std::unique_ptr<Framebuffer> m_framebuffer;
    std::unique_ptr<Framebuffer> m_posframebuffer;

    std::unique_ptr<Renderbuffer> m_renderbuffer;

    u32 m_width;
    u32 m_height;

    void SetupGBuffer();

    void SetupPosProcess();
};

