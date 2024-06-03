#include "gl.h"

<<<<<<< HEAD
namespace gr {

const char* get_enum_name(GLenum err) {
    #define GETENUMNAME(e) case e: return #e;
    switch (err) {
        /* ERROR */
        GETENUMNAME(GL_INVALID_ENUM);
        GETENUMNAME(GL_INVALID_VALUE);
        GETENUMNAME(GL_INVALID_OPERATION);
        GETENUMNAME(GL_INVALID_FRAMEBUFFER_OPERATION);
        GETENUMNAME(GL_OUT_OF_MEMORY);
        GETENUMNAME(GL_STACK_UNDERFLOW);
        GETENUMNAME(GL_STACK_OVERFLOW);
        GETENUMNAME(GL_CONTEXT_LOST);
    default:
        return "Unknown";
    }
    #undef GETENUMNAME
}

void check_erros_opengl(const std::string &name) {
    u32 numErrors = 0;
    GLenum err;

    while ((err = glGetError()) != GL_NO_ERROR) {
        numErrors++;
        std::cout << "Opengl error: " << name << "(" << numErrors << ")" << std::endl;
    }
}
}
=======
#include <iostream>

namespace grr {

    void gl_gleck_function(const std::string& name) {
        int numErrors = 0;
        GLenum err;

        while ((err = glGetError()) != GL_NO_ERROR) {
            numErrors++;

            std::runtime_error("Opengl Error: " + name + " " + std::to_string(numErrors));
        }
    }
};
>>>>>>> 7c9a1b2 (	new file:   .gitignore)
