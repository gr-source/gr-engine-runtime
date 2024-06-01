#include "gShader.h"

#include "grende.h"

#include <GL/glew.h>

gShader* gShader::instance = nullptr;

gShader::gShader() : m_index(0) {}

gShader *gShader::Create(const std::vector<std::string> &fragments, const std::vector<std::string> &vertex) {
    gShader* shader = new gShader();
    shader->m_index = GL_CALL(glCreateProgram());
    
    return shader;
}

void gShader::destroy() {
    delete this;
}
