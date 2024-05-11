#pragma once

#include <iostream>
#include <cassert>
#include <string>
#include <array>

#include <GL/glew.h>

#include "types.h"

#include "log.h"

inline std::array<std::string, 9> errNames = {
    "Unknown OpenGL error",
    "GL_INVALID_ENUM",
    "GL_INVALID_VALUE",
    "GL_INVALID_OPERATION",
    "GL_INVALID_FRAMEBUFFER_OPERATION",
    "GL_OUT_OF_MEMORY",
    "GL_STACK_UNDERFLOW",
    "GL_STACK_OVERFLOW",
    "GL_CONTEXT_LOST" 
};

inline void check_for_opengl_errors(const std::string& functionName) {
    int numErrors = 0;
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        numErrors++;
        int errNum = 0;
        switch (err) {
        case GL_INVALID_ENUM:
            errNum = 1;
            break;
        case GL_INVALID_VALUE:
            errNum = 2;
            break;
        case GL_INVALID_OPERATION:
            errNum = 3;
            break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            errNum = 4;
            break;
        case GL_OUT_OF_MEMORY:
            errNum = 5;
            break;
        case GL_STACK_UNDERFLOW:
            errNum = 6;
            break;
        case GL_STACK_OVERFLOW:
            errNum = 7;
            break;
        case GL_CONTEXT_LOST:
            errNum = 8;
            break;
        }
        throw std::runtime_error(Log::red + "OpenGL ERROR: " + errNames[errNum] + " " + functionName + Log::none);
    }
}

#if OPENGL_DEBUG
#define GL_CHECK(func) func; check_for_opengl_errors(#func)
#else
#define GL_CHECK(func) func
#endif

#define CHECK_FUNCTION(index) if (index == -1) return;

#define GSetViewport(width, height) GL_CHECK(glViewport(0, 0, width, height))

#define GSetBlitFramebuffer(srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, mask, filter) GL_CHECK(glBlitFramebuffer(srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, mask, filter))

#define GEnableDepthTest() glEnable(GL_DEPTH_TEST)
#define GEnableFrameBufferSRGB() glEnable(GL_FRAMEBUFFER_SRGB)
#define GCullFaceFront() glCullFace(GL_FRONT)
#define GCullFaceBack() glCullFace(GL_BACK);

#define GDisableDepthTest() glDisable(GL_DEPTH_TEST)
#define GDisableFrameBufferSRGB() glDisable(GL_FRAMEBUFFER_SRGB)

#define GRENDERER3D GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT
#define GRENDERER2D GL_COLOR_BUFFER_BIT
#define GRENDERERDEPTH GL_DEPTH_BUFFER_BIT

#define GClearColor(color) glClearColor(color.r, color.g, color.b, color.a)

#define GRender(view) glClear(view)

#define GDisableTexture(index) glActiveTexture(GL_TEXTURE0 + index); glBindTexture(GL_TEXTURE_2D, 0)

