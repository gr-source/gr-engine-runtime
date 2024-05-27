#include "gl.h"

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