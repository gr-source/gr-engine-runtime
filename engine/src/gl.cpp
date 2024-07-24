#include "gl.h"

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

void check_for_opengl_errors(const std::string& functionName) {
    int numErrors = 0;
    GLenum err;

    std::stringstream ss;
    while ((err = glGetError()) != GL_NO_ERROR) {
        numErrors++;
        ss << std::endl;
        ss << "========== " << functionName << " ==========" << std::endl;
        ss << "[ " << numErrors << " ] " << get_enum_name(err) << std::endl;
        throw std::runtime_error(Log::red + ss.str() + Log::none);
    }
}

