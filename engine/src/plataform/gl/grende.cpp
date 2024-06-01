#include "grende.h"

#include <iostream>

#include <GL/glew.h>

#include "../../Color.h"

void SetRenderState(RenderState state, void *value) {
    switch (state) {
    case GR_BACKGROUND_COLOR: {
        Color* color = (Color*)value;
        glClearColor(color->r, color->g, color->b, color->a);
        break;
    }
    default:
        std::cout << "Invalid type: " << getRenderStateName(state) << std::endl;
        break;
    }
}

void SetRenderState(RenderState state, RenderState value) {
    switch (state) {
    case GR_BACKGROUND: {
        GLbitfield filter = 0;
        if (value & GR_DEPTH_BUFFER) {
            filter |= GL_DEPTH_BUFFER_BIT;
        }
        if (value & GR_COLOR_BUFFER) {
            filter |= GL_COLOR_BUFFER_BIT;
        }
        glClear(filter);
        break;
    }
    default:
        std::cout << "Invalid type: " << getRenderStateName(state) << std::endl;
    }
}

const std::string getRenderStateName(RenderState state) {
    #define GET_ENUM_NAME(e) case e: return #e
    switch (state) {
    GET_ENUM_NAME(GR_BACKGROUND);
    GET_ENUM_NAME(GR_BACKGROUND_COLOR);
    GET_ENUM_NAME(GR_COLOR_BUFFER);
    GET_ENUM_NAME(GR_DEPTH_BUFFER);
    GET_ENUM_NAME(GR_FALSE);
    GET_ENUM_NAME(GR_TRUE);
    default:
        break;
    }
    #undef GET_ENUM_NAME
}
