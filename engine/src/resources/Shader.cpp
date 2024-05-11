#include "Shader.h"

#include "../Global.h"

#include "Resources.h"

#define CHECK_PROGMA \
    if (programID == -1) { \
        programID = glCreateProgram(); \
    } \

Shader::Shader(const std::string& path) : path(path), programID(-1) {}

Shader::Shader(const std::string& vertexFile, const std::string& fragmentFile) : programID(-1) {
    loadFromFile(vertexFile, eVERTEX);
    loadFromFile(fragmentFile, eFRAGMENT);
}

void Shader::loadSource(const std::string& data, u8 type) {
    CHECK_PROGMA
    switch (type) {
    case eFRAGMENT: {
        createShader(data.c_str(), GL_FRAGMENT_SHADER);
        break;
    }
    case eVERTEX: {
        createShader(data.c_str(), GL_VERTEX_SHADER);
        break;
    }
    case eGEOMETRY: {
        createShader(data.c_str(), GL_GEOMETRY_SHADER);
        break;
    }
    default:
        assert(false && "Error type do not loadede!");
    }
}

void Shader::loadFromFile(const std::string& path, u8 type) {
    CHECK_PROGMA
    
    std::ifstream is(path);
    if (!is.is_open()) {
        std::cerr << "Shader do not openned(" << (type == eFRAGMENT ? "frag)" : "vert)") << std::endl;
        return;
    }

    std::string data((std::istreambuf_iterator<char>(is)), std::istreambuf_iterator<char>());

    const char* content = data.c_str();

    is.close();

    switch (type) {
    case eFRAGMENT: {
        createShader(content, GL_FRAGMENT_SHADER);
        break;
    }
    case eVERTEX: {
        createShader(content, GL_VERTEX_SHADER);
        break;
    }
    case eGEOMETRY: {
        createShader(content, GL_GEOMETRY_SHADER);
        break;
    }
    default:
        std::cerr << "Error type do not loadede!" << std::endl;
        break;
    }
}

void Shader::bind() {
    GL_CHECK(glUseProgram(programID));
}

template <>
void Shader::setUniform(const std::string& name, const Matrix4x4& value) {
    GLuint location =  GL_CHECK(glGetUniformLocation(programID, name.c_str()));
    if (location != GL_INVALID_INDEX) {
        GL_CHECK(glUniformMatrix4fv(location, 1, GL_FALSE, value.data));
    } else {
        throw std::runtime_error("Error object not found: " + name);
    }
}

template <>
void Shader::setUniform(const std::string& name, const Matrix3x3& value) {
    GLuint location =  GL_CHECK(glGetUniformLocation(programID, name.c_str()));
    if (location != GL_INVALID_INDEX) {
        GL_CHECK(glUniformMatrix3fv(location, 1, GL_FALSE, value.data));
    } else {
        throw std::runtime_error("Error object not found: " + name);
    }
}

template <>
void Shader::setUniform(const std::string& name, const Vector3& value) {
    GLuint location =  GL_CHECK(glGetUniformLocation(programID, name.c_str()));
    if (location != GL_INVALID_INDEX) {
        GL_CHECK(glUniform3fv(location, 1, value.data));
    } else {
        throw std::runtime_error("Error object not found: " + name);
    }
}

template <>
void Shader::setUniform(const std::string& name, const Color& value) {
    GLuint location =  GL_CHECK(glGetUniformLocation(programID, name.c_str()));
    if (location != GL_INVALID_INDEX) {
        GL_CHECK(glUniform4fv(location, 1, value.data));
    } else {
        throw std::runtime_error("Error object not found: " + name);
    }
}

template <>
void Shader::setUniform(const std::string& name, const float& value) {
    GLuint location =  GL_CHECK(glGetUniformLocation(programID, name.c_str()));
    if (location != GL_INVALID_INDEX) {
        GL_CHECK(glUniform1f(location, value));
    } else {
        throw std::runtime_error("Error object not found: " + name);
    }
}

template <>
void Shader::setUniform(const std::string& name, const int& value) {
    GLuint location = GL_CHECK(glGetUniformLocation(programID, name.c_str()));
    if (location != GL_INVALID_INDEX) {
        GL_CHECK(glUniform1i(location, value));
    } else {
        // throw std::runtime_error("Error object not found: " + name);
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

std::string Shader::loadFile(const std::string& path) const {
    std::ifstream file(path);
    if (!file.is_open()) {
        return nullptr;
    }
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    
    file.close();
    return content;
}

void Shader::checkError(GLuint& shader, const std::string& path) {
    GLint status;
    GL_CHECK(glGetShaderiv(shader, GL_COMPILE_STATUS, &status));
    if (status != GL_TRUE) {
        GLint length;
        GL_CHECK(glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length));

        GLint type;
        GL_CHECK(glGetShaderiv(shader, GL_SHADER_TYPE, &type));

        std::string logMessage(length, '\0');

        GL_CHECK(glGetShaderInfoLog(shader, logMessage.size(), nullptr, logMessage.data()));

        std::cout << "Shader error(" << path << ")";
        switch (type) {
        case GL_FRAGMENT_SHADER:
            std::cout << "(GL_FRAGMENT_SHADER):" << std::endl;
            break;
        case GL_VERTEX_SHADER:
            std::cout << "(GL_VERTEX_SHADER):" << std::endl;
            break;
        default:
            break;
        }
        std::cout << logMessage << std::endl;
    }
}

void Shader::createShader(const char* data, GLenum type) {
    /* cria um obejto shader vazio retorna zero caso tenha erros */
    GLuint shaderID = GL_CHECK(glCreateShader(type));

    /* preenche o objeto vazio */
    GL_CHECK(glShaderSource(shaderID, 1, &data, nullptr));

    /* compila o objeto */
    GL_CHECK(glCompileShader(shaderID));
    checkError(shaderID, "null");

    /* anexe o objeto ao programa principal  */
    GL_CHECK(glAttachShader(programID, shaderID));

    /* ligue o objeto ao programa */
    GL_CHECK(glLinkProgram(programID));

    /* delete o objeto nao sera util pos ja esta no programa do shader */
    GL_CHECK(glDeleteShader(shaderID));
}



