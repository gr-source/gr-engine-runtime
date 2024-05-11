#pragma once

#include "../gl.h"

class Shader {
public:
    enum : u8 {
        eNONE     = 1 << 1,
        eFRAGMENT = 1 << 2,
        eVERTEX   = 1 << 3,
        eGEOMETRY = 1 << 4,
    };

    Shader(const std::string& path);

    Shader(const std::string& vertexFile, const std::string& fragmentFile);
    ~Shader();

    /* carregue a string source: frag | vert | geo */
    void loadSource(const std::string& data, u8 type);

    /* carregue uma source shader do arguivo: .frag | .vert | .geo */
    void loadFromFile(const std::string& path, u8 type);

    /* use o shader */
    void bind();

    /* desuse o shader */
    void unbind();

    template <typename T>
    void setUniform(const std::string& name, const T& value);
    
    GLuint programID;

    bool isvalid() const;
    
    const std::string& getpath() const;

protected:
    std::string loadFile(const std::string& path) const;

    void checkError(GLuint& shaders, const std::string& path);

    void createShader(const char* data, GLenum type);

    std::string path;

    u8 m_flags;
};

