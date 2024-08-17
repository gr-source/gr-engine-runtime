#include "resources.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../core/stb/stb_image.h"

#include "../filesystem/filesystem.h"

#include "../eventManager.h"
#include "../console.hpp"
#include "../settings.h"

#include <filesystem>
#include <future>

u32 Resources::m_processCount = 0;

/***************** Texture *****************/
std::unordered_map<std::string, std::unique_ptr<Texture>> Resources::m_textures;
std::list<ProcessLoader<Texture>> Resources::m_textureLoader;
std::mutex Resources::m_textureMutex;

/***************** SHADER *****************/
std::unordered_map<std::string, std::unique_ptr<Shader>> Resources::m_shaders;
std::list<ProcessLoader<Shader>> Resources::m_shaderLoader;
std::mutex Resources::m_shaderMutex;

/***************** Shader *****************/
std::unordered_map<std::string, std::unique_ptr<grr::gShader>> Resources::s_shaderList;
std::list<std::tuple<std::string, grr::gShader *>> Resources::s_shaderFromLoader;
std::mutex Resources::s_shaderMutex;

/***************** Texture *****************/
std::unordered_map<std::string, std::unique_ptr<grr::gTexture>> Resources::s_textureList;
std::list<std::tuple<std::string, grr::gTexture *>> Resources::s_textureFromLoader;
std::mutex Resources::s_textureMutex;

template <>
Shader* Resources::Load<Shader>(const std::string& path, const std::function<void(Shader*)>& callback) {
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
        m_shaderLoader.emplace_back(m_shader, callback);
    }

    return m_shader;
}

template <>
Texture* Resources::Load<Texture>(const std::string& path, const std::function<void(Texture*)>& callback) {
    if (!Filesystem::Exists(path)) {
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

grr::gTexture *Resources::LoadTextureL(const std::string &path) {
    if (!std::filesystem::exists(path)) { 
        Console::Print(LogInfo::ERROR, "[ Resources::LoadTexture ] Falied to load texture: " + path);
        return nullptr;
    }

    auto it = s_textureList.find(path);
    if (it != s_textureList.end()) {
        Console::Print(LogInfo::WARNING, "The texture has already been loaded.");
        return (*it).second.get();
    }

    {
        std::lock_guard<std::mutex> lock(s_textureMutex);
        auto it = std::find_if(s_textureFromLoader.begin(), s_textureFromLoader.end(), [path](const auto &pair) {
            return std::get<std::string>(pair) == path;
        });
        if (it != s_textureFromLoader.end()) {
            return std::get<grr::gTexture *>((*it));
        }
    }

    auto m_texture = new grr::gTexture();
    if (!m_texture) {
        Console::Print(LogInfo::ERROR, "[ Resources::LoadTexture ] Falied to create texture.");
        return nullptr;
    }
    
    {
        std::lock_guard<std::mutex> lock(s_textureMutex);
        s_textureFromLoader.emplace_back(path, m_texture);
    }
    return m_texture;
}

grr::gShader *Resources::LoadShaderL(const std::string &path) {
    if (!Filesystem::Exists(path + ".frag")) {
        Console::Print(LogInfo::ERROR, "File do not found: " + path + ".frag");
        return nullptr;
    }

    if (!Filesystem::Exists(path + ".vert")) {
        Console::Print(LogInfo::ERROR, "File do not found: " + path + ".vert");
        return nullptr;
    }

    auto it = s_shaderList.find(path);
    if (it != s_shaderList.end()) {
        return (*it).second.get();
    }

    grr::gShader *shader = new grr::gShader();
    if (!shader) {
        Console::Print(LogInfo::ERROR, "Falied to create shader.");
        return nullptr;
    }

    {
        std::lock_guard<std::mutex> lock(s_shaderMutex);
        s_shaderFromLoader.emplace_back(path, shader);
    }
    return shader;
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
        break;
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

        EventManager::AddEvent(setup_texture, process, width, height, channel, pixels);
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

        std::string fcontent = Filesystem::GetFileContent(process.data->getpath() + ".frag");
        if (fcontent.empty()) {
            lock.lock();
            m_shaderLoader.pop_front();
            continue;
        }

        /* load vertex shader string */
        std::string vcontent = Filesystem::GetFileContent(process.data->getpath() + ".vert");
        if (vcontent.empty()) {
            lock.lock();
            m_shaderLoader.pop_front();
            continue;
        }

        /* load geometry shader string */
        const std::string gcontent = Filesystem::GetFileContent(process.data->getpath() + ".geo");
        if (gcontent.empty()) {
            Log::warning() << "[ Shader ] geometry shader file do not open" << Log::end;
        }

        EventManager::AddEvent(setup_shader, process, fcontent, vcontent, gcontent);
        lock.lock();
        m_shaderLoader.pop_front();
    }
}

void Resources::SetupTexture(grr::gTexture *texture, int width, int height, int ch, void *pixels, std::string path) {
    s_textureList.emplace(path, std::unique_ptr<grr::gTexture>(texture));

    grr::TextureFormat format;

    switch (ch) {
    case 1:
        // return Texture::eRED;
    case 2:
        // return Texture::eRG;
    case 3:
        format = grr::TextureFormat_SRGB;
        break;
    case 4:
        format = grr::TextureFormat_SRGBA;
        break;
    default:
        assert(false && "[ Resources::getTextureType ] typo invalido");
        break;
    }

    grr::gTexture::PushTexture(texture, width, height, grr::gTextureFlags_MipMaps, format, pixels);

    stbi_image_free(pixels);

    Console::Print(LogInfo::INFO, "Texture loaded: " + path);
}

void Resources::LoadTextureF() {
    std::unique_lock<std::mutex> lock(s_textureMutex);
    while (!s_textureFromLoader.empty()) {
        auto [path, texture] = s_textureFromLoader.front();
        lock.unlock();

        int width, height, ch;
        auto pixels = stbi_load(path.c_str(), &width, &height, &ch, 0);
        if (!pixels) {
            Console::Print(LogInfo::ERROR, "Failed to load texture: " + path);

            delete texture;

            lock.lock();
            s_textureFromLoader.pop_front();
            continue;
        }

        EventManager::AddEvent(Resources::SetupTexture, texture, width, height, ch, (void *)pixels, path);

        lock.lock();
        s_textureFromLoader.pop_front();
    }
}

void Resources::SetupShader(grr::gShader *shader, const std::string &frag, const std::string &vert) {
    Console::Print(LogInfo::INFO, "Shader loadedd sucessfull.");

    const char *fShader = frag.c_str();
    const char *vShader = vert.c_str();

    grr::gShader::Create(shader, &fShader, &vShader);
}

void Resources::LoadShaderF() {
    std::unique_lock<std::mutex> lock(s_shaderMutex);
    while (!s_shaderFromLoader.empty()) {
        auto [path, shader] = s_shaderFromLoader.front();
        lock.unlock();

        // Processing shader

        std::string frag = path + ".frag";
        std::string vert = path + ".vert";

        // Loading Fragment shader
        Console::Print(LogInfo::WARNING, "Loading FragmentShader: " + frag + " - " + vert);

        auto fragSource = Filesystem::GetFileContent(frag);
        if (fragSource.empty()) {
            Console::Print(LogInfo::ERROR, "Falied to load: " + frag);
            delete shader;

            lock.lock();
            s_shaderFromLoader.pop_front();
            continue;
        }

        // Loading vertex shader

        Console::Print(LogInfo::WARNING, "Loading VertexShader: " + vert);
        auto vertSource = Filesystem::GetFileContent(vert);
        if (vertSource.empty()) {
            Console::Print(LogInfo::ERROR, "Falied to load: " + vert);
            delete shader;

            lock.lock();
            s_shaderFromLoader.pop_front();
            continue;
        }

        // Setup Shader
        s_shaderList.emplace(path, std::unique_ptr<grr::gShader>(shader));

        EventManager::AddEvent(Resources::SetupShader, shader, fragSource, vertSource);

        lock.lock();
        s_shaderFromLoader.pop_front();
    }
}

void Resources::OnThread() {
    LoadShader();

    LoadTexture();

    // Script Loader
    LoadShaderF();

    LoadTextureF();
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

const grm::size Resources::GetTextureCount() {
    return s_textureList.size();
}

const grm::size Resources::GetShaderCount() {
    return s_shaderList.size();
}
