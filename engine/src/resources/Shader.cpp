#include "Shader.h"

#include "../Global.h"

#include "Resources.h"

#define CHECK_PROGMA \
    if (programID == -1) { \
        programID = glCreateProgram(); \
    } \

Shader::Shader(const std::string& path) : path(path), programID(-1) {}

void Shader::loadsource(const char** data, u32 count, u8 type) {
    CHECK_PROGMA
    switch (type) {
    case eFRAGMENT: {
        createshader(data, count, GL_FRAGMENT_SHADER);
        break;
    }
    case eVERTEX: {
        createshader(data, count, GL_VERTEX_SHADER);
        break;
    }
    case eGEOMETRY: {
        createshader(data, count, GL_GEOMETRY_SHADER);
        break;
    }
    default:
        assert(false && "Error type do not loadede!");
    }
}

void Shader::RegisterUniform(const std::string &name, const u32 &count, u8 type) {
    Uniform* location = getUniformIndex(name);
    if (!location) {
        u32 index =  GL_CHECK(glGetUniformLocation(programID, name.c_str()));
        if (index != GL_INVALID_INDEX)  {
            auto& uniform = m_uniforms[name];
            uniform.location = index;
            uniform.count = count;
            uniform.type = type;
            uniform.data = nullptr;
        } else {
            std::cout << "Unform invalid: " << name  << " (" << path << ")" << std::endl;
        }
    }
}

void Shader::bind() {
    GL_CHECK(glUseProgram(programID));
}

template <>
void Shader::setUniform(const std::string& name, u32 num, const Matrix4x4* value) {
    Uniform* uniform = getUniformIndex(name);
    if (uniform) {
        GL_CHECK(glUniformMatrix4fv(uniform->location, num, GL_FALSE, reinterpret_cast<const GLfloat*>(value)));
    }
}

template <>
void Shader::setUniform(const std::string& name, const Matrix4x4& value) {
    Uniform* uniform = getUniformIndex(name);
    if (uniform) {
        GL_CHECK(glUniformMatrix4fv(uniform->location, 1, GL_FALSE, value.data));
    }
}

template <>
void Shader::setUniform(const std::string& name, const Matrix3x3& value) {
    Uniform* location = getUniformIndex(name);
    if (location) {
        GL_CHECK(glUniformMatrix3fv(location->location, 1, GL_FALSE, value.data));
    }
}

template <>
void Shader::setUniform(const std::string& name, const Vector3& value) {
    Uniform* location = getUniformIndex(name);
    if (location) {
        GL_CHECK(glUniform3fv(location->location, 1, value.data));
    }
}

template <>
void Shader::setUniform(const std::string& name, const Color& value) {
    Uniform* location = getUniformIndex(name);
    if (location) {
        GL_CHECK(glUniform4fv(location->location, 1, value.data));
    }
}

template <>
void Shader::setUniform(const std::string& name, const float& value) {
    Uniform* location = getUniformIndex(name);
    if (location) {
        GL_CHECK(glUniform1f(location->location, value));
    }
}

template <>
void Shader::setUniform(const std::string& name, const int& value) {
    Uniform* location = getUniformIndex(name);
    if (location) {
        GL_CHECK(glUniform1i(location->location, value));
    }
}

void Shader::unbind() {
    glUseProgram(0);
}

Shader::~Shader() {
    glDeleteProgram(programID);
}

bool Shader::isvalid() const {
    return programID != -1;
}

const std::string &Shader::getpath() const {
    return path;
}

const u32 &Shader::getID() const {
    return programID;
}

void Shader::checkerrors(GLuint& shader, const std::string& path, bool compile) {
    GLint status;
    if (compile) {
        GL_CHECK(glGetShaderiv(shader, GL_COMPILE_STATUS, &status));
        if (status != GL_TRUE) {
            GLint length;
            GL_CHECK(glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length));

            GLint type;
            GL_CHECK(glGetShaderiv(shader, GL_SHADER_TYPE, &type));

            std::string logMessage(length, '\0');
            GL_CHECK(glGetShaderInfoLog(shader, logMessage.size(), nullptr, logMessage.data()));

            std::cout << "Failed to compile shader - " << path;
            switch (type) {
            case GL_FRAGMENT_SHADER:
                std::cout << ".frag" << std::endl << std::endl;
                break;
            case GL_VERTEX_SHADER:
                std::cout << ".vert" << std::endl << std::endl;
                break;
            default:
                break;
            }
            std::cout << logMessage << std::endl;
        }
        return;
    }

    GL_CHECK(glGetProgramiv(shader, GL_LINK_STATUS, &status));
    if (status != GL_TRUE) {
        std::string logMessage(512, '\0');

        GL_CHECK(glGetProgramInfoLog(shader, logMessage.size(), nullptr, logMessage.data()));
        std::cout << "Failed to link shader program - " << path << std::endl << std::endl;
        std::cout << logMessage << std::endl;
    }
}

void Shader::createshader(const char** data, u32 count, GLenum type) {
    GLuint shaderID = GL_CHECK(glCreateShader(type));

    GL_CHECK(glShaderSource(shaderID, count, data, nullptr));
    GL_CHECK(glCompileShader(shaderID));
    checkerrors(shaderID, std::filesystem::path(path).filename(), true);


    GL_CHECK(glAttachShader(programID, shaderID));
    GL_CHECK(glLinkProgram(programID));
    checkerrors(programID, std::filesystem::path(path).filename(), false);

    GL_CHECK(glDeleteShader(shaderID));
}

Shader::Uniform* Shader::getUniformIndex(const std::string &name) {
    auto it = m_uniforms.find(name);
    if (it != m_uniforms.end()) {
        return &(*it).second;
    }
    return nullptr;
}
