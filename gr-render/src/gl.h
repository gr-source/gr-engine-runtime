#pragma once

<<<<<<< HEAD
#include "common.h"

#include <GL/glew.h>

namespace gr {
    const char* get_enum_name(GLenum err);

    void check_erros_opengl(const std::string& name);

    #define GL_CALL(func) func; check_erros_opengl(#func)
}
=======
#include <GL/glew.h>

#include <string>

namespace grr {

    #define GL_CHECK(func) func; gl_gleck_function(#func)

    void gl_gleck_function(const std::string& name);

};
>>>>>>> 7c9a1b2 (	new file:   .gitignore)
