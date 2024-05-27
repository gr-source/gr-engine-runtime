#pragma once

#include <iostream>
#include <cassert>
#include <sstream>
#include <string>
#include <array>

#include <GL/glew.h>

#include "types.h"

#include "log.h"


const char* get_enum_name(GLenum err);

void check_for_opengl_errors(const std::string& functionName);

#define GL_CHECK(func) func; check_for_opengl_errors(#func)

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

