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

#include "texture2D.h"
#include "sprite.h"
#include "Shader.h"

#include "../core/stb/stb_image.h"

template <typename T>
struct ProcessLoader {
    T* data;
    std::function<void(T*)> callback;

    ProcessLoader(T*& _data, std::function<void(T*)> _callback) : data(_data), callback(_callback) {}
};


class Resources {
public:
    static void Initialize();
    
    template <typename T>
    static T* Load(const std::string& path, const std::function<void(T*)>& callback = nullptr);

    static Texture2D* Load(const std::string& path, const std::function<void(Texture2D*)>& callback, u32 internalFormat, u32 format, u32 type, u32 filter, bool hasMipmap);

    static u32 getTextureType(int ch);

    static GLenum getTextureTypeSRGB(int ch);

    static void OnThread();

    static const u32 getspritecount();

    static const u32 gettexturecount();

    static const u32 getshadercount();

    static u32 m_processCount;

    static const char* gettextureformatname(u32 format);

private:
    /***************** SPRITE *****************/
    static std::unordered_map<std::string, std::unique_ptr<Sprite>> m_sprites;
    static std::list<ProcessLoader<Sprite>> m_spriteLoader;
    static std::condition_variable m_spritecv;
    static std::mutex m_spriteMutex;
    static bool m_spriteReady;

    /***************** TEXTURE2D *****************/
    static std::unordered_map<std::string, std::unique_ptr<Texture2D>> m_textures;
    static std::list<ProcessLoader<Texture2D>> m_textureLoader;
    static std::condition_variable m_texturecv;
    static std::mutex m_textureMutex;
    static bool m_textureReady;

    /***************** SHADER *****************/
    static std::unordered_map<std::string, std::unique_ptr<Shader>> m_shaders;
    static std::list<ProcessLoader<Shader>> m_shaderLoader;
    static std::condition_variable m_shadercv;
    static std::mutex m_shaderMutex;
    static bool m_shaderReady;

    static void setup_sprite(ProcessLoader<Sprite>& process, const int width, const int height, const int channel, stbi_uc*& data);

    static void LoadSprite();

    static void setup_texture(ProcessLoader<Texture2D>& process, const int width, const int height, const int channel, stbi_uc*& data);

    static void LoadTexture();

    static void setup_shader(ProcessLoader<Shader>& process, const std::string& fcontent, const std::string& vcontent, const std::string& gcontent);

    static void LoadShader();
};

