#pragma once

#include "../gl.h"

#include <unordered_map>

class Shader {
public:
    enum : u8 {
        eNONE     = 1 << 1,
        eFRAGMENT = 1 << 2,
        eVERTEX   = 1 << 3,
        eGEOMETRY = 1 << 4,

        eMAT4x4   = 2 << 1,
        eMAT3x3   = 2 << 2,
        eINT      = 2 << 3,
        eFLOAT    = 2 << 4,
        eVECTOR3  = 2 << 5,
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

    Shader(const std::string& vertexFile, const std::string& fragmentFile);
    ~Shader();

    /* carregue a string source: frag | vert | geo */
    void loadSource(const std::string& data, u8 type);

    /* carregue uma source shader do arguivo: .frag | .vert | .geo */
    void loadFromFile(const std::string& path, u8 type);

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

protected:
    std::string loadFile(const std::string& path) const;

    void checkError(GLuint& shaders, const std::string& path);

    void createShader(const char* data, GLenum type);

    std::string path;

    u32 programID;

    u8 m_flags;

    std::unordered_map<std::string, Uniform> m_uniforms;

    Uniform* getUniformIndex(const std::string& name);
};

