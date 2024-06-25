#pragma once

#include "../gl.h"

#include <unordered_map>

class Shader {
public:
    enum : u32 {
        eNONE     = 1 << 1,
        eFRAGMENT = GL_FRAGMENT_SHADER,
        eVERTEX   = GL_VERTEX_SHADER,
        eGEOMETRY = 1 << 4,

        eMAT4x4   = 2 << 1,
        eMAT3x3   = 2 << 2,
        eINT      = 2 << 3,
        eFLOAT    = 2 << 4,
        eVector3  = 2 << 5,
        eVECTOR4  = 2 << 6,
        eCOLOR    = 3 << 1,
    };

    struct Uniform {
        u32 location;
        u8 type;
        u32 count;
        void* data;

        const bool isvalid() const {
            return location != -1;
        }

        Uniform(const u32& location = -1, const u8& type = eNONE, const u32& count = 1, void* data = nullptr) : location(location), type(type), count(count), data(data) {}
    };

    Shader(const std::string& path);
    ~Shader();

    static Shader *Create(const char *fragment, const char *vertex);

    /* carregue a string source: frag | vert | geo */
    void loadsource(const char** data, u32 count, u32 type);

    void RegisterUniform(const std::string& name, const u32& count, u8 type);

    /* use o shader */
    void bind();

    /* desuse o shader */
    void unbind();

    template <typename T>
    void setUniform(const std::string& name, const T& value);

    template <typename T>
    void setUniform(const std::string& name, u32 num, const T* value);

    bool isvalid() const;
    
    const std::string& getpath() const;

    const u32& getID() const;

    void destroy();

    static Shader* CurrentShader;

protected:
    void checkerrors(GLuint& shaders, const std::string& path, bool compile);

    void createshader(const char** data, u32 count, GLenum type);

    std::string path;

    u32 programID;

    u8 m_flags;

    std::unordered_map<std::string, Uniform> m_uniforms;

    Uniform* getUniformIndex(const std::string& name);
};

