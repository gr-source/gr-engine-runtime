#pragma once

#include <GL/glew.h>

#include "../../common.h"

#include "renderbuffer.h"

#include "../../resources/texture.h"

class Framebuffer {
public:
    enum : u32 {
        eCOLOR_ATTACHMENT0 = GL_COLOR_ATTACHMENT0,
        eCOLOR_ATTACHMENT1 = GL_COLOR_ATTACHMENT1,
        eCOLOR_ATTACHMENT2 = GL_COLOR_ATTACHMENT2,
        eCOLOR_ATTACHMENT3 = GL_COLOR_ATTACHMENT3,
        eCOLOR_ATTACHMENT4 = GL_COLOR_ATTACHMENT4,
        eCOLOR_ATTACHMENT5 = GL_COLOR_ATTACHMENT5,
        eDEPTH_ATTACHMENT  = GL_DEPTH_ATTACHMENT,
        eSTENCIL_ATTACHMENT = GL_STENCIL_ATTACHMENT,
        eDEPTH_STENCIL_ATTACHMENT = GL_DEPTH_STENCIL_ATTACHMENT,

        eTexture2D = GL_TEXTURE_2D,
        eTextureSample2D = GL_TEXTURE_2D_MULTISAMPLE,
    };

    Framebuffer();
    ~Framebuffer();

    void bind();

    void unbind();

    void read();
    
    void draw();

    void none();

    void SetTexture2D(u32 attachment, u32 textTarget, Texture* texture);

    void SetRenderbuffer(u32 attachment, Renderbuffer* renderbuffer);
    
    const u32& getID() const;

protected:
    u32 m_fbo;
};



