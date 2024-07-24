#pragma once

#include "internal/framebuffer.h"

#include <memory>

#include <gr-math/gmath.h>
#include <GL/glew.h>

#include "../resources/texture.h"
#include "../resources/cubemap.h"

#include "../types.h"

enum class FrameBufferStatus : int {
    ERROR = 0,
    SUCESS = 1,
    RESIZE = 2
};

class FrameBuffer {
public:
    FrameBufferStatus state;

    FrameBuffer(u32 width, u32 height) : m_width(width), m_height(height) {}

    void setviewport(const Vector2& scale) { glViewport(0, 0, scale.x, scale.y);}

    virtual void bind() {}
    virtual void unbind() {}
    virtual void resize(const Vector2& scale) {}
    virtual GLuint gettexture() { return -1; }

    virtual ~FrameBuffer() {}

    u32 m_width;
    u32 m_height;
};

