#pragma once

#include "../filesystem/filesystem.h"

#include <condition_variable>
#include <functional>
#include <memory>
#include <string>
#include <vector>
#include <mutex>
#include <list>
#include <map>

#include <gr-render/gRender.h>

#include "texture.h"
#include "shader.h"

#include "../core/stb/stb_image.h"

template <typename T>
struct ProcessLoader {
    T* data;
    std::function<void(T*)> callback;

    ProcessLoader(T* _data, std::function<void(T*)> _callback) : data(_data), callback(_callback) {}
};

struct TextureArgs {
    u32 internalFormat;
    u32 format;
    u32 type;
    u32 filter;
    bool hasMipmap;
};

class Resources {
public:
    static void Initialize();
    
    template <typename T>
    static T* Load(const std::string& path, const std::function<void(T*)>& callback = nullptr);

    template <typename... Args>
    static Texture* LoadTexture(const std::string& path, Args&&... args, TextureArgs textureArgs={Texture::eNONE, Texture::eNONE, Texture::eNONE, Texture::eNONE, true});

    template <typename... Args>
    static Shader* LoadShader(const std::string& path, Args&&... args);

    static grr::gTexture *LoadTextureL(const std::string &path);

    static grr::gShader *LoadShaderL(const std::string &path);

    static u32 getTextureType(int ch);

    static GLenum getTextureTypeSRGB(int ch);

    static void OnThread();

    static const u32 gettexturecount();

    static const u32 getshadercount();

    static u32 m_processCount;

    static const char* gettextureformatname(u32 format);

    static const grm::size GetTextureCount();

    static const grm::size GetShaderCount();

private:
    /***************** Texture *****************/
    static std::unordered_map<std::string, std::unique_ptr<Texture>> m_textures;
    static std::list<ProcessLoader<Texture>> m_textureLoader;
    static std::mutex m_textureMutex;

    /***************** SHADER *****************/
    static std::unordered_map<std::string, std::unique_ptr<Shader>> m_shaders;
    static std::list<ProcessLoader<Shader>> m_shaderLoader;
    static std::mutex m_shaderMutex;

    /***************** Shader *****************/
    static std::unordered_map<std::string, std::unique_ptr<grr::gShader>> s_shaderList;
    static std::list<std::tuple<std::string, grr::gShader *>> s_shaderFromLoader;
    static std::mutex s_shaderMutex;

    /***************** Texture *****************/
    static std::unordered_map<std::string, std::unique_ptr<grr::gTexture>> s_textureList;
    static std::list<std::tuple<std::string, grr::gTexture *>> s_textureFromLoader;
    static std::mutex s_textureMutex;

    static void setup_texture(ProcessLoader<Texture>& process, const u32 width, const u32 height, const int channel, stbi_uc*& pixels);

    static void LoadTexture();

    static void setup_shader(ProcessLoader<Shader>& process, const std::string& fcontent, const std::string& vcontent, const std::string& gcontent);

    static void LoadShader();

    static void SetupTexture(grr::gTexture *texture, int width, int height, int ch, void *pixels);

    static void LoadTextureF();

    static void SetupShader(grr::gShader *shader, const std::string &frag, const std::string &vert);

    static void LoadShaderF();
};

template <typename... Args>
inline Texture *Resources::LoadTexture(const std::string &path, Args &&...args, TextureArgs textureArgs) {
    if (!Filesystem::Exists(path)) {
        Log::error() << "File texture do not open: " << path << Log::end;
        return nullptr;
    }

    auto it = m_textures.find(Filesystem::path(path).filename());
    if (it != m_textures.end()) {
        return (*it).second.get();
    }

    Texture* texture = new Texture(path);
    texture->m_internalformat = textureArgs.internalFormat;
    texture->m_format = textureArgs.format;
    texture->m_filter = textureArgs.filter;
    texture->m_type = textureArgs.type;
    texture->m_hasMipmap = textureArgs.hasMipmap;

    ++m_processCount;

    {
        std::lock_guard<std::mutex> lock(m_textureMutex);
        m_textureLoader.emplace_back(texture, std::bind(std::forward<Args>(args)..., texture));
    }

    return texture;
}

template <typename... Args>
inline Shader *Resources::LoadShader(const std::string &path, Args &&...args) {
    if (!Filesystem::Exists(path + ".frag")) {
        Log::error() << "[ Shader ] fragment shader file not found (" << path << ".frag)" << Log::end;
        return nullptr;
    }
    if (!Filesystem::Exists(path + ".vert")) {
        Log::error() << "[ Shader ] vertex shader file not found (" << path << ".vert)" << Log::end;
        return nullptr;
    }

    auto it = m_shaders.find(Filesystem::path(path).filename());
    if (it != m_shaders.end()) {
        return (*it).second.get();
    }

    Shader* m_shader = new Shader(path);

    ++m_processCount;
    
    {
        std::lock_guard<std::mutex> lock(m_shaderMutex);
        m_shaderLoader.emplace_back(m_shader, std::bind(std::forward<Args>(args)..., m_shader));
    }

    return m_shader;
}
