#include "resources.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../core/stb/stb_image.h"

#include "../filesystem/filesystem.h"
#include "../eventManager.h"
#include "../settings.h"

#include <future>

/***************** Texture *****************/
std::unordered_map<std::string, std::unique_ptr<Texture>> Resources::m_textures;
std::list<ProcessLoader<Texture>> Resources::m_textureLoader;
std::mutex Resources::m_textureMutex;

/***************** SHADER *****************/
std::unordered_map<std::string, std::unique_ptr<Shader>> Resources::m_shaders;
std::list<ProcessLoader<Shader>> Resources::m_shaderLoader;
std::mutex Resources::m_shaderMutex;

u32 Resources::m_processCount = 0;

template <>
Shader* Resources::Load<Shader>(const std::string& path, const std::function<void(Shader*)>& callback) {
    if (!Filesystem::exists(path + ".frag")) {
        Log::error() << "[ Shader ] fragment shader file not found (" << path << ".frag)" << Log::end;
        return nullptr;
    }
    if (!Filesystem::exists(path + ".vert")) {
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
        m_shaderLoader.emplace_back(m_shader, callback);
    }

    return m_shader;
}

template <>
Texture* Resources::Load<Texture>(const std::string& path, const std::function<void(Texture*)>& callback) {
    if (!Filesystem::exists(path)) {
        Log::error() << "[ Texture ] file not found (" << path << ")" << Log::end;
        return nullptr;
    }

    auto it = m_textures.find(Filesystem::path(path).filename());
    if (it != m_textures.end()) {
        return (*it).second.get();
    }

    Texture* m_texture = new Texture(path);

    ++m_processCount;

    {
        std::lock_guard<std::mutex> lock(m_textureMutex);
        m_textureLoader.emplace_back(m_texture, callback);
    }

    return m_texture;
}

void Resources::Initialize() {
    stbi_set_flip_vertically_on_load(true);
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

/***************** TEXTURE LOAD ****************/
void Resources::setup_texture(ProcessLoader<Texture>& process, const u32 width, const u32 height, const int channel, stbi_uc*& pixels) {
    u32 internalFormat = (process.data->m_internalformat != Texture::eNONE ? process.data->m_internalformat : Resources::getTextureTypeSRGB(channel));
    u32 format = (process.data->m_format != Texture::eNONE ? process.data->m_format : Resources::getTextureType(channel));
    u32 type = (process.data->m_type != Texture::eNONE ? process.data->m_type : Texture::eBYTE);

    process.data->resize(width, height, internalFormat, format, Texture::eTRILINEAR, type, true);
    process.data->apply(pixels);

    stbi_image_free(pixels);
    pixels = nullptr;

    m_textures.emplace(Filesystem::path(process.data->getpath()).filename(), std::unique_ptr<Texture>(process.data));

    --m_processCount;

    if (process.callback != nullptr) {
        process.callback(process.data);
    }
}

void Resources::LoadTexture() {
    std::unique_lock<std::mutex> lock(m_textureMutex);
    while (!m_textureLoader.empty()) {
        auto& process = m_textureLoader.front();
        lock.unlock();

        int width = 0, height = 0, channel = 0;
        stbi_uc* pixels = stbi_load(process.data->getpath().c_str(), &width, &height, &channel, 0);

        if (!pixels) {
            Log::error() << "[Texture] pixels not loaded" << Log::end;

            delete process.data;
            process.data = nullptr;

            stbi_image_free(pixels);

            lock.lock();
            m_textureLoader.pop_front();
            continue;
        }

        EventManager::addEvent(setup_texture, process, width, height, channel, pixels);
        lock.lock();
        m_textureLoader.pop_front();
    }
}

/***************** SHADER LOAD ****************/
void Resources::setup_shader(ProcessLoader<Shader>& process, const std::string& fcontent, const std::string& vcontent, const std::string& gcontent) {
    if (!fcontent.empty()) {
        const char* str = fcontent.c_str();
        process.data->loadsource(&str, 1, Shader::eFRAGMENT);
    } else {
        Log::error() << "[ Resources::setup_shader ] Fatal error f" << Log::end;

        delete process.data;
        process.data = nullptr;
        return;
    }

    if (!vcontent.empty()) {
        const char* str = vcontent.c_str();
        process.data->loadsource(&str, 1, Shader::eVERTEX);
    } else {
        Log::error() << "[ Resources::setup_shader ] Fatal error v" << Log::end;

        delete process.data;
        process.data = nullptr;
        return;
    }

    if (!gcontent.empty()) {
        const char* str = gcontent.c_str();
        process.data->loadsource(&str, 1, Shader::eGEOMETRY);
    }

    m_shaders.emplace(Filesystem::path(process.data->getpath()).filename(), std::unique_ptr<Shader>(process.data));

    --m_processCount;
    
    if (process.callback != nullptr) {
        process.callback(process.data);
    }
}

void Resources::LoadShader() {
    std::unique_lock<std::mutex> lock(m_shaderMutex);
    while (!m_shaderLoader.empty()) {
        auto& process = m_shaderLoader.front();
        lock.unlock();

        std::string fcontent = Filesystem::getfilecontent(process.data->getpath() + ".frag");
        if (fcontent.empty()) {
            lock.lock();
            m_shaderLoader.pop_front();
            continue;
        }

        /* load vertex shader string */
        std::string vcontent = Filesystem::getfilecontent(process.data->getpath() + ".vert");
        if (vcontent.empty()) {
            lock.lock();
            m_shaderLoader.pop_front();
            continue;
        }

        /* load geometry shader string */
        const std::string gcontent = Filesystem::getfilecontent(process.data->getpath() + ".geo");
        if (gcontent.empty()) {
            Log::warning() << "[ Shader ] geometry shader file do not open" << Log::end;
        }

        EventManager::addEvent(setup_shader, process, fcontent, vcontent, gcontent);
        lock.lock();
        m_shaderLoader.pop_front();
    }
}

void Resources::OnThread() {
    LoadShader();

    LoadTexture();
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
