#pragma once

#include <GL/glew.h>

#include <string>

namespace grr {

    #define GL_CHECK(func) func; gl_gleck_function(#func)

    void gl_gleck_function(const std::string& name);

};