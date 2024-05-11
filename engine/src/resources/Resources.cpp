#include "Resources.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../core/stb/stb_image.h"

#include "../eventManager.h"
#include "../settings.h"

#include <future>

/***************** SPRITE *****************/
std::unordered_map<std::string, std::unique_ptr<Sprite>> Resources::m_sprites;
std::list<ProcessLoader<Sprite>> Resources::m_spriteLoader;
std::mutex Resources::m_spriteMutex;
std::condition_variable Resources::m_spritecv;
bool Resources::m_spriteReady = false;

/***************** TEXTURE2D *****************/
std::unordered_map<std::string, std::unique_ptr<Texture2D>> Resources::m_textures;
std::list<ProcessLoader<Texture2D>> Resources::m_textureLoader;
std::mutex Resources::m_textureMutex;
std::condition_variable Resources::m_texturecv;
bool Resources::m_textureReady = false;

/***************** SHADER *****************/
std::unordered_map<std::string, std::unique_ptr<Shader>> Resources::m_shaders;
std::list<ProcessLoader<Shader>> Resources::m_shaderLoader;
std::mutex Resources::m_shaderMutex;
std::condition_variable Resources::m_shadercv;
bool Resources::m_shaderReady = false;

u32 Resources::m_processCount = 0;

template <>
Shader* Resources::Load<Shader>(const std::string& path, SucessFunc callback) {
    if (!std::filesystem::exists(path + ".frag")) {
        ERROR("[ Shader ] fragment shader file not found (" + path + ".frag)");
        return nullptr;
    }
    if (!std::filesystem::exists(path + ".vert")) {
        ERROR("[ Shader ] vertex shader file not found (" + path + ".vert)");
        return nullptr;
    }

    auto it = m_shaders.find(path);
    if (it != m_shaders.end()) {
        return (*it).second.get();
    }

    Shader* m_shader = new Shader(path);

    ++m_processCount;
    
    {
        std::lock_guard<std::mutex> lock(m_shaderMutex);
        m_shaderLoader.emplace_back(m_shader, callback);
    }

    return m_shader;
}

template <>
Texture2D* Resources::Load<Texture2D>(const std::string& path, SucessFunc callback) {
    if (!std::filesystem::exists(path)) {
        ERROR("[ Texture2D ] file not found (" + path + ")");
        return nullptr;
    }

    auto it = m_textures.find(path);
    if (it != m_textures.end()) {
        return (*it).second.get();
    }

    Texture2D* m_texture = new Texture2D(-1, path);

    {
        std::lock_guard<std::mutex> lock(m_textureMutex);
        m_textureLoader.emplace_back(m_texture, callback);
    ++m_processCount;
    }

    return m_texture;
}

void Resources::Initialize() {
    // stbi_set_flip_vertically_on_load(true);
}

Texture2D *Resources::Load(const std::string& path, SucessFunc callback, u32 internalFormat, u32 format, u32 type, u32 filter, bool hasMipmap) {
    if (!std::filesystem::exists(path)) {
        ERROR("[ Texture2D ] file not found (" + path + ")");
        return nullptr;
    }

    auto it = m_textures.find(path);
    if (it != m_textures.end()) {
        return (*it).second.get();
    }

    Texture2D* m_texture = new Texture2D(-1, path);
    m_texture->m_internalformat = internalFormat;
    m_texture->m_format = format;
    m_texture->m_type = type;
    m_texture->m_filter = filter;
    m_texture->m_hasMipmap = hasMipmap;

    ++m_processCount;

    {
        std::lock_guard<std::mutex> lock(m_textureMutex);
        m_textureLoader.emplace_back(m_texture, callback);
    }

    return m_texture;
}

u32 Resources::getTextureType(int ch) {
    switch (ch) {
    case 1:
        return Texture::eRED;
    case 2:
        return Texture::eRG;
    case 3:
        return Texture::eRGB;
    case 4:
        return Texture::eRGBA;
    default:
        assert(false && "[ Resources::getTextureType ] typo invalido");
    }
}

GLenum Resources::getTextureTypeSRGB(int ch) {
    switch (ch) {
    case 1:
    case 2:
    case 3:
        return GL_SRGB;
    case 4:
        return GL_SRGB_ALPHA;
    default:
        return GL_SRGB;
    }
}

/***************** SPRITE LOAD *****************/
template <>
Sprite* Resources::Load<Sprite>(const std::string& path, SucessFunc callback) {
    if (!std::filesystem::exists(path)) {
        ERROR("[ Sprite ] file not found (" + path + ")");
        return nullptr;
    }

    auto it = m_sprites.find(path);
    if (it != m_sprites.end()) {
        return (*it).second.get();
    }

    Sprite* m_sprite = new Sprite(-1, path);

    ++m_processCount;

    {
        std::lock_guard<std::mutex> lock(m_spriteMutex);
        m_spriteLoader.emplace_back(m_sprite, callback);
    }

    return m_sprite;
}

void Resources::setup_sprite(ProcessLoader<Sprite>& process, const int width, const int height, const int channel, stbi_uc*& data) {
    process.data->resize(width, height, Resources::getTextureType(channel), Resources::getTextureType(channel), Texture::eBILINEAR, Texture::eBYTE, false);
    process.data->apply(data);

    m_sprites[process.data->getpath()] = std::unique_ptr<Sprite>(process.data);

    --m_processCount;

    {
        std::lock_guard<std::mutex> lock(m_spriteMutex);
        m_spriteReady = true;

        m_spritecv.notify_one();
    }

    if (process.callback != nullptr) {
        process.callback();
    }
}

void Resources::LoadSprite() {
    std::unique_lock<std::mutex> lock(m_spriteMutex);
    while (!m_spriteLoader.empty()) {
        auto& m_process = m_spriteLoader.front();

        int width = 0, height = 0, channel = 0;
        stbi_uc* data = stbi_load(m_process.data->getpath().c_str(), &width, &height, &channel, 0);
        if (!data) {
            ERROR("[ Sprite ] data do not loaded");

            delete m_process.data;
            m_process.data = nullptr;

            m_spriteLoader.pop_front();

            return;
        }

        EventManager::addEvent(setup_sprite, m_process, width, height, channel, data);

        {
            m_spritecv.wait(lock, []{ return m_spriteReady || !Settings::IsEngineRunning;});

            stbi_image_free(data);
            data = nullptr;

            m_spriteReady = false;

            m_spriteLoader.pop_front();

        }
    }
}


/***************** TEXTURE LOAD ****************/
void Resources::setup_texture(ProcessLoader<Texture2D>& process, const int width, const int height, const int channel, stbi_uc*& data) {
    u32 internalFormat = (process.data->m_internalformat != Texture::eNONE ? process.data->m_internalformat : Resources::getTextureTypeSRGB(channel));
    u32 format = (process.data->m_format != Texture::eNONE ? process.data->m_format : Resources::getTextureType(channel));
    u32 type = (process.data->m_type != Texture::eNONE ? process.data->m_type : Texture::eBYTE);

    process.data->resize(width, height, internalFormat, format, Texture::eTRILINEAR, type, true);
    process.data->apply(data);

    m_textures[process.data->getpath()] = std::unique_ptr<Texture2D>(process.data);

    --m_processCount;

    {
        std::lock_guard<std::mutex> lock(m_textureMutex);
        m_textureReady = true;

        m_texturecv.notify_one();
    }

    if (process.callback != nullptr) {
        process.callback();
    }
}

void Resources::LoadTexture() {
    std::unique_lock<std::mutex> lock(m_textureMutex);
    while (!m_textureLoader.empty()) {
        auto& process = m_textureLoader.front();

        int width = 0, height = 0, channel = 0;
        stbi_uc* data = stbi_load(process.data->getpath().c_str(), &width, &height, &channel, 0);
        if (!data) {
            ERROR("[ Texture2D ] data do not loaded");

            delete process.data;
            process.data = nullptr;
            
            m_textureLoader.pop_front();

            return;
        }

        EventManager::addEvent(setup_texture, process, width, height, channel, data);

        {
            m_texturecv.wait(lock, []{return m_textureReady || !Settings::IsEngineRunning;});

            stbi_image_free(data);
            data = nullptr;

            m_textureReady = false;

            m_textureLoader.pop_front();
        }
        
    }
}

template <>
void Resources::Remove(Texture2D* resouce) {
    auto m_texture = m_textures.find(resouce->getpath());
    if (m_texture != m_textures.end()) {
        m_textures.erase(m_texture);
    } else {
        std::cout << "[ Resources::Remove ] textura nao encontrada" << std::endl;
    }
}

/***************** SHADER LOAD ****************/
void Resources::setup_shader(ProcessLoader<Shader>& process, const std::string& fcontent, const std::string& vcontent, const std::string& gcontent) {
    if (!fcontent.empty()) {
        process.data->loadSource(fcontent, Shader::eFRAGMENT);
    } else {
        ERROR("[ Resources::setup_shader ] Fatal error f");

        delete process.data;
        process.data = nullptr;
        return;
    }

    if (!vcontent.empty()) {
        process.data->loadSource(vcontent, Shader::eVERTEX);
    } else {
        ERROR("[ Resources::setup_shader ] Fatal error v");

        delete process.data;
        process.data = nullptr;
        return;
    }

    if (!gcontent.empty()) {
        process.data->loadSource(gcontent, Shader::eGEOMETRY);
    }

    m_shaders[process.data->getpath()] = std::unique_ptr<Shader>(process.data);

    --m_processCount;
    
    {
        std::lock_guard<std::mutex> lock(m_shaderMutex);
        m_shaderReady = true;

        m_shadercv.notify_one();
    }

    if (process.callback != nullptr) {
        process.callback();
    }
}

void Resources::LoadShader() {
    std::unique_lock<std::mutex> lock(m_shaderMutex);
    while (!m_shaderLoader.empty()) {
        auto& process = m_shaderLoader.front();

        std::string fcontent, vcontent, gcontent;

        /* load fragment shader string */
        std::ifstream file(process.data->getpath() + ".frag");
        if (!file.is_open()) {
            ERROR("[ Shader ] fragment shader file do not open");

            delete process.data;
            process.data = nullptr;

            m_shaderLoader.pop_front();

            return;
        }
        fcontent = std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

        file.close();


        /* load vertex shader string */
        file.open(process.data->getpath() + ".vert");
        if (!file.is_open()) {
            ERROR("[ Shader ] vertex shader file do not open");

            delete process.data;
            process.data = nullptr;

            m_shaderLoader.pop_front();

            return;
        }
        
        vcontent = std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

        file.close();

        /* load geometry shader string */
        file.open(process.data->getpath() + ".geo");
        if (file.is_open()) {
            gcontent = std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        } else {
            WARNING("[ Shader ] geometry shader file do not open");
        }
        file.close();

        EventManager::addEvent(setup_shader, process, fcontent, vcontent, gcontent);

        {
            m_shadercv.wait(lock, []{return m_shaderReady || !Settings::IsEngineRunning;});

            m_shaderReady = false;

            m_shaderLoader.pop_front();
        }
    }
}

void Resources::OnThread() {
    stbi_set_flip_vertically_on_load(false);
    LoadSprite();

    LoadShader();

    stbi_set_flip_vertically_on_load(true);
    LoadTexture();
}

const u32 Resources::getspritecount() {
    return m_sprites.size();
}

const u32 Resources::gettexturecount() {
    return m_textures.size();
}

const u32 Resources::getshadercount() {
    return m_shaders.size();
}

const char *Resources::gettextureformatname(u32 format) {
    switch (format) {
    case Texture::eRG:
        return "RG";
    case Texture::eRED:
        return "RED";
    case Texture::eRGB:
        return "RGB";
    case Texture::eRGB16F:
        return "RGB16";
    case Texture::eRGB32F:
        return "RGB32";
    case Texture::eRGBA:
        return "RGBA";
    case Texture::eRGBA16F:
        return "RGBA16";
    case Texture::eRGBA32F:
        return "RGBA32";
    case Texture::eSRBG:
        return "eSRGB";
    case Texture::eSRBG8:
        return "eSRGB8";
    case Texture::eSRBG8_APHA8:
        return "eSRGB8 ALPHA8";
    case Texture::eSRBG_APHA:
        return "eSRGB ALPHA";
    default:
        return "<ERROR>";
    }
}
