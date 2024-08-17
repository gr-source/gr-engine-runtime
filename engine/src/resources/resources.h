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
    
    static grr::gTexture *LoadTexture(const std::string &path);

    static grr::gShader *LoadShader(const std::string &path);

    static GameObject *LoadGameObject(const std::string &path);

    static void OnThread();

    static u32 m_processCount;

    static const grm::size GetTextureCount();

    static const grm::size GetShaderCount();

private:
    /***************** Shader *****************/
    static std::unordered_map<std::string, std::unique_ptr<grr::gShader>> s_shaderList;
    static std::list<std::tuple<std::string, grr::gShader *>> s_shaderFromLoader;
    static std::mutex s_shaderMutex;

    /***************** Texture *****************/
    static std::unordered_map<std::string, std::unique_ptr<grr::gTexture>> s_textureList;
    static std::list<std::tuple<std::string, grr::gTexture *>> s_textureFromLoader;
    static std::mutex s_textureMutex;

    static void SetupTexture(grr::gTexture *texture, int width, int height, int ch, void *pixels, std::string path);

    static void ProcessTexture();

    static void SetupShader(grr::gShader *shader, const std::string &frag, const std::string &vert);

    static void ProcessShader();
};


