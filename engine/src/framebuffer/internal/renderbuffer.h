#pragma once

#include "../../gl.h"

class Renderbuffer {
public:
    enum : u32 {
        /* internal format */
        eDEPTH24_STENCIL8 = GL_DEPTH24_STENCIL8,
        eDEPTH_COMPONENT = GL_DEPTH_COMPONENT,
        eDEPTH_COMPONENT16 = GL_DEPTH_COMPONENT16,
        eDEPTH_COMPONENT24 = GL_DEPTH_COMPONENT24,
        eDEPTH_COMPONENT32 = GL_DEPTH_COMPONENT32
    };

    Renderbuffer();
    ~Renderbuffer();

    void bind();

    void unbind();

    GLuint getRBO();

    void bufferStorage(u32 internalFomat, u32 width, u32 height);

    void bufferStorageSampler(u32 internalFomat, u32 samples,u32 width, u32 height);
protected:
    GLuint m_rbo;
};

