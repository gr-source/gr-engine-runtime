#include "assetsManager.hpp"

#include "core/file/stream.hpp"
#include "core/file/file.hpp"

#include "core/animation/animation.hpp"

#include "core/threadpool.hpp"
#include "core/scene.hpp"

#include "ecs/gameObject.h"

#include "script/meshFilter.hpp"
#include "script/meshRender.hpp"
#include "script/transform.hpp"
#include "script/animator.hpp"

#include "console.hpp"
#include "settings.h"

#include "event.hpp"

#include "scriptManager/scriptManager.hpp"

#include <filesystem>
#include <cassert>
#include <future>

#include <gr-render/gError.h>
#include <gr-render/gTexture.h>

#define STB_IMAGE_IMPLEMENTATION
#include "../core/stb/stb_image.h"

grm::u32 AssetsManager::m_processCount = 0;

/***************** Shader *****************/
std::unordered_map<std::string, std::unique_ptr<grr::gShader>> AssetsManager::s_shaderList;
std::list<ShaderHandle> AssetsManager::s_shaderFromLoader;
std::mutex AssetsManager::s_shaderMutex;

/***************** Texture *****************/
std::unordered_map<std::string, std::unique_ptr<grr::gTexture>> AssetsManager::m_textureMap;
std::unordered_map<std::string, TextureHandle *> AssetsManager::m_mapHandleTexture;
std::list<TextureHandle> AssetsManager::m_textureHandle;
std::mutex AssetsManager::m_mutexTexture;

/***************** Animation *****************/
std::unordered_map<std::string, std::unique_ptr<Animation>> AssetsManager::s_animationList;
std::list<std::tuple<std::string, Animation *>> AssetsManager::s_animationFromLoader;
std::mutex AssetsManager::s_animatioMutex;

/***************** GameObject *****************/
std::list<GameObjectHandle> AssetsManager::s_gameObjectFromLoader;
std::mutex AssetsManager::s_gameObjectMutex;

/***************** Mesh *****************/
std::unordered_map<std::string, std::unique_ptr<Mesh>> AssetsManager::s_meshList;
std::mutex AssetsManager::m_meshMutex;

/***************** Material *****************/
// std::unordered_map<std::string, std::vector<Material>> AssetsManager::m_materialMap;

void AssetsManager::Initialize() {
    stbi_set_flip_vertically_on_load(true);
}

void AssetsManager::Shutdown() {
    // clear assets
    m_processCount = 0;

    /***************** Shader *****************/
    s_shaderList.clear();
    s_shaderFromLoader.clear();

    /***************** Texture *****************/
    m_textureMap.clear();
    // multithread
    m_mapHandleTexture.clear();
    m_textureHandle.clear();

    /***************** Animation *****************/
    s_animationList.clear();
    s_animationFromLoader.clear();

    /***************** GameObject *****************/
    s_gameObjectFromLoader.clear();

    /***************** Mesh *****************/
    s_meshList.clear();

    /***************** Material *****************/
    // m_materialMap.clear();
}

Animation *AssetsManager::LoadAnimation(const std::string &path) {
    if (!File::Exists(path)) {
        Console::log(LogInfo::ERROR, "Falied to open animation: " + path);
        return nullptr;
    }
    auto it = s_animationList.find(path);
    if (it != s_animationList.end()) {
        return &(*(*it).second);
    }

    {
        std::lock_guard<std::mutex> lock(s_animatioMutex);
        auto it = std::find_if(s_animationFromLoader.begin(), s_animationFromLoader.end(), [&path](auto &tuple) {
            return std::get<std::string>(tuple) == path;
        });
        if (it != s_animationFromLoader.end()) {
            return std::get<Animation *>((*it));
        }
    }

    auto animation = new Animation();
    {
        std::lock_guard<std::mutex> lock(s_animatioMutex);
        s_animationFromLoader.emplace_back(path, animation);
    }
    return animation;
}

grr::gShader *AssetsManager::FindShader(const std::string &name) {
    auto it = s_shaderList.find(name);
    if (it == s_shaderList.end()) {
        return nullptr;
    }
    return &(*it->second);
}

// ********** Mesh ********** //

Mesh *AssetsManager::FindMesh(const std::string &name) {
    Mesh *m_mesh = nullptr;
    {
        std::lock_guard<std::mutex> lock(m_meshMutex);
        auto it = s_meshList.find(name);
        if (it == s_meshList.end()) {
            return nullptr;
        }
        m_mesh = &(*it->second);
    }
    return m_mesh;
}

void AssetsManager::DeleteMesh(Mesh *target) {
    if (!target) {
        return;
    }
    
    std::unique_lock<std::mutex> lock(m_meshMutex);
    auto it = s_meshList.find(target->getName());
    if (it == s_meshList.end()) {
        return;
    }
    s_meshList.erase(it);
    lock.unlock();
}

std::unordered_map<std::string, std::unique_ptr<Mesh>> &AssetsManager::GetMeshListMap() {
    return s_meshList;
}

/***************** Texture Process ****************/
void AssetsManager::SetupTexture(TextureHandle handle, int width, int height, int ch, void *pixels) {
    std::lock_guard<std::mutex> lock(m_mutexTexture);
    auto it = m_textureMap.find(handle.m_path);
    if (it != m_textureMap.end()) {
        handle.invoke(&(*it->second));
        return;
    }
    auto texture = grr::gTexture::Create();

    grr::TextureFormat format;

    {

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

        grr::gTexture::Bind(texture);
        grr::gTexture::UpdateTexture(width, height, grr::gTextureFlags_MipMaps | grr::gTextureFlags_Filter_Bilinear, format, pixels);

        stbi_image_free(pixels);
    }

    m_textureMap.emplace(handle.m_path, std::unique_ptr<grr::gTexture>(texture));
   
    handle.invoke(texture);

    LOG_INFO("[ AssetsManager::Texture ] Texture loaded: " + handle.m_path);
}

void AssetsManager::ProcessTexture() {
    std::unique_lock<std::mutex> lock(m_mutexTexture);
    while (!m_textureHandle.empty()) {
        auto handle = m_textureHandle.front();
        m_textureHandle.pop_front();

        auto path = handle.m_path;
        m_mapHandleTexture.erase(path);

        int width, height, ch;
        auto pixels = stbi_load(path.c_str(), &width, &height, &ch, 0);
        if (!pixels) {
            LOG_ERROR("[ AssetsManager::Texture ] Failed to load texture: " + path);
            continue;
        }
        lock.unlock();

        Event::Add(&AssetsManager::SetupTexture, handle, width, height, ch, (void *)pixels);

        lock.lock();
    }
}

/***************** Shader Process ****************/
void AssetsManager::SetupShader(ShaderHandle handle, const std::string &frag, const std::string &vert) {
    auto it = s_shaderList.find(handle.m_name);
    if (it != s_shaderList.end()) {
        handle.invoke(&(*it->second));
        return;
    }

    const char *fShader = frag.c_str();
    const char *vShader = vert.c_str();

    auto shader = grr::gShader::Create(&fShader, &vShader);

    if (CheckShaderError(shader, handle.m_name)) {
        delete shader;
        return;
    }

    s_shaderList.emplace(handle.m_name, std::unique_ptr<grr::gShader>(shader));

    handle.invoke(shader);

    LOG_INFO("[ AssetsManager::Shader ] Shader loadedd sucessfull: " + handle.m_name);
}

bool AssetsManager::CheckShaderError(grr::gShader *shader, const std::string &name) {
    if (shader->isValid()) {
        return false;
    }

    std::stringstream iss(grr::gError::GetError());

    Console::log(LogInfo::ERROR, "Shader error: " + name);
    std::string line;
    while (std::getline(iss, line)) {
        Console::log(LogInfo::ERROR, line);
    }

    return true;
}

void AssetsManager::ProcessShader() {
    std::unique_lock<std::mutex> lock(s_shaderMutex);
    while (!s_shaderFromLoader.empty()) {
        auto handle = s_shaderFromLoader.front();
        s_shaderFromLoader.pop_front();
        lock.unlock();

        // Processing shader
        auto fragSource = File::GetFileContent(handle.m_frag);
        if (fragSource.empty()) {
            Console::log(LogInfo::ERROR, "Falied to shader: " + handle.m_name + " Fragment: " + handle.m_frag);

            lock.lock();
            continue;
        }

        // Loading vertex shader
        auto vertSource = File::GetFileContent(handle.m_vert);
        if (vertSource.empty()) {
            Console::log(LogInfo::ERROR, "Falied to shader: " + handle.m_name + " Vertex: " + handle.m_vert);

            lock.lock();
            continue;
        }

        // lock.unlock();
        Event::Add(AssetsManager::SetupShader, handle, fragSource, vertSource);

        lock.lock();
    }
}

#include "filmbox/fbxScene.hpp"

void AssetsManager::SetupAnimation(const std::string &path, FbxAnimation *temp, Animation *animation) {
    animation->setName(temp->getName());
    animation->setDuration(temp->getDuration());
    animation->setTick(temp->getTick());

    for (const auto &channel : temp->getChannels()) {
        auto &keyframe = animation->addkeyframe(channel.getName(), channel.getName());

        for (auto &keyScale : channel.getKeysScale()) {
            keyframe.addKeyScale(keyScale.time, keyScale.value);
        }

        for (auto &keyRotation : channel.getKeysRotation()) {
            keyframe.addKeyRotation(keyRotation.time, keyRotation.value);
        }

        for (auto &keyPosition : channel.getKeysPosition()) {
            keyframe.addKeyPosition(keyPosition.time, keyPosition.value);
        }
    }

    s_animationList.emplace(path, std::unique_ptr<Animation>(animation));
    
    delete temp;
}

void AssetsManager::doAnimationInBackground(const std::string &path, Animation *target) {
    /*
    Streaming stream(path, std::ios_base::in | std::ios_base::binary);
    if (!stream.is_open()) {
        Console::log(LogInfo::ERROR, "Falied to open animation: " + path);
        return;
    }

    gHeader pHeader;
    Stream::read<gHeader>(&pHeader, 1);

    gAnimationHeader pAnimationHeader;
    Stream::read<gAnimationHeader>(&pAnimationHeader, 1);

    auto fbxAnimation = new FbxAnimation(pAnimationHeader.m_name, pAnimationHeader.m_duration, pAnimationHeader.m_tick);

    grm::size sChannel;
    Stream::read<grm::size>(&sChannel, 1);

    std::vector<gChannel> pChannels(sChannel);

    for (auto i=0;i<sChannel;++i) {
        auto &pChannel = pChannels[i];

        Stream::read<char>(pChannel.m_name, 256);

        // gen key scale
        Stream::read<grm::size>(&pChannel.m_keyScaleCount, 1);
        pChannel.m_keyScale = new gKeyScale[pChannel.m_keyScaleCount];
        Stream::read<gKeyScale>(pChannel.m_keyScale, pChannel.m_keyScaleCount);

        //gen key rotation
        Stream::read<grm::size>(&pChannel.m_keyRotationCount, 1);
        pChannel.m_keyRotation = new gKeyRotation[pChannel.m_keyRotationCount];
        Stream::read<gKeyRotation>(pChannel.m_keyRotation, pChannel.m_keyRotationCount);

        // gen key position
        Stream::read<grm::size>(&pChannel.m_keyPositionCount, 1);
        pChannel.m_keyPosition = new gKeyPosition[pChannel.m_keyPositionCount];
        Stream::read<gKeyPosition>(pChannel.m_keyPosition, pChannel.m_keyPositionCount);
    }

    for (auto &channel : pChannels) {
        auto &keyframe = fbxAnimation->addChannel(channel.m_name);

        for (auto i=0;i<channel.m_keyScaleCount;++i) {
            auto &keyScale = channel.m_keyScale[i];

            keyframe.addKeyScale(keyScale.time, keyScale.value);
        }

        for (auto i=0;i<channel.m_keyRotationCount;++i) {
            auto &keyRotation = channel.m_keyRotation[i];

            keyframe.addKeyRotation(keyRotation.time, keyRotation.value);
        }

        for (auto i=0;i<channel.m_keyPositionCount;++i) {
            auto &keyPosition = channel.m_keyPosition[i];

            keyframe.addKeyPosition(keyPosition.time, keyPosition.value);
        }
    }

    // releaser memory
    for (auto i=0;i<sChannel;++i) {
        auto &pChannel = pChannels[i];

        // gen key scale
        delete[] pChannel.m_keyScale;

        //gen key rotation
        delete[] pChannel.m_keyRotation;

        // gen key position
        delete[] pChannel.m_keyPosition;
    }

    Stream::close();

    Event::Add(&AssetsManager::SetupAnimation, path, fbxAnimation, target);
    */
}

void AssetsManager::ProcessAnimation() {
    std::unique_lock<std::mutex> lock(s_animatioMutex);
    while (!s_animationFromLoader.empty()) {
        auto &&[path, animation] = s_animationFromLoader.front();
        lock.unlock();

        doAnimationInBackground(path, animation);

        lock.lock();
        s_animationFromLoader.pop_front();
    }
}

void AssetsManager::SetupGameObject(std::condition_variable &cond, GameObjectHandle &handle, Transform *&out, gModel model, Mesh *mesh, std::vector<gMaterial> materials, Transform *parent) {
    // transformaçoes
    GameObject *gameObject = nullptr;
    {
        std::lock_guard<std::mutex> lock(s_gameObjectMutex);
        gameObject = GameObject::Create(model.m_name, "unttaged");
        if (!handle.m_gameObject) {
            handle.m_gameObject = gameObject;
        }
    }

    auto transform = gameObject->getTransform();

    transform->setLocalScale(model.m_scale);
    transform->setLocalRotation(model.m_rotation);
    transform->setLocalPosition(model.m_position);

    if (parent)
        transform->setParent(parent);

    if (mesh) {
        auto meshFilter = gameObject->addComponent<MeshFilter>();
        meshFilter->setMesh(mesh);

        auto meshRender = gameObject->addComponent<MeshRender>();

        for (auto &material : materials) {
            if (!material.m_textures) {
                throw std::runtime_error("Texture ptr is invalid.");
            }
            
            auto material_ptr = meshRender->addMaterial(material.m_name, material.m_color);

            for (int i=0;i<material.m_textureCount;++i) { 
                auto& texture = material.m_textures[i];
                
                AssetsManager::LoadTexture(texture.m_path, &AssetsManager::SetupTextureMaterial, material_ptr, i);
            }
        }
    }

    {
        std::lock_guard<std::mutex> lock(s_gameObjectMutex);
        out = transform;
    }
    cond.notify_one();
}

void AssetsManager::ReadChildren(Transform *parent, GameObjectHandle &handle, Stream *stream) {
    gModel pModel;
    if (!stream->readS(&pModel, 1)) {
        return;
    }

    grm::u32 meshCount;
    if (!stream->readS(&meshCount, 1)) {
        Console::log(LogInfo::ERROR, "Failed to read mesh count to file.");
        return;
    }

    std::vector<gMesh> meshes(meshCount);
    for (auto &mesh : meshes) {
        mesh.deserialize(stream);
    }

    Animator *m_animator = nullptr;
    Mesh *m_mesh = nullptr;

    // adiciona o mesh componenet
    if (!meshes.empty()) {
        Mesh mesh;

        std::string meshName;

        grm::u32 verticeCount = 0;
        for (auto &pMesh : meshes) {
            meshName += pMesh.m_name;

            for (int i=0;i<pMesh.m_verticeCount;i++) {
                auto &vertice = pMesh.m_vertices[i];

                mesh.addVertex(vertice.position, vertice.normal, vertice.texCoord, vertice.mIdx);

                if (!pMesh.m_hasBone) {
                    continue;
                }
                
                auto& boneWeight = mesh.addBoneWeight();

                float norm = vertice.weight[0] + vertice.weight[1] + vertice.weight[2] + vertice.weight[3];

                for (auto k=0;k<4;++k) {
                    boneWeight.insert<int>(k, vertice.boneIndex[k]);
                    boneWeight.insert<float>(k, norm > 0.0f ? vertice.weight[k] / norm : vertice.weight[k]);
                }
            }

            for (int i=0;i<pMesh.m_indicesCount;i++) {
                mesh.addIndice(verticeCount + pMesh.m_indices[i]);
            }
            verticeCount += pMesh.m_verticeCount;

            if (!pMesh.m_hasBone) {
                continue;
            }

            if (!m_animator) {
                // m_animator = &gameObject->addComponent<Animator>();
            }

            for (auto i=0;i<pMesh.m_boneCount;++i) {
                auto &pBone = pMesh.m_bones[i];

                m_animator->addBone(pBone.m_name, Bone{pBone.m_name, pBone.m_position, pBone.m_scale, pBone.m_rotation, pBone.m_id});
            }

        }

        {
            m_mesh = AssetsManager::FindMesh(meshName);
            if (!m_mesh) {
                mesh.setName(meshName);

                m_mesh = AssetsManager::CreateMesh(meshName, mesh);
            }
        }
    }

    grm::u32 mSize;
    if (!stream->readS(&mSize, 1)) {
        return;
    }

    std::vector<gMaterial> m_materials(mSize);
    for (auto &material : m_materials) {
        stream->readS(material.m_name, 256);
        stream->readS(&material.m_color, 1);
        stream->readS(&material.m_roughness, 1);
        stream->readS(&material.m_metallic, 1);
        stream->readS(&material.m_ambientOcclusion, 1);

        stream->readS(&material.m_textureCount, 1);

        material.m_textures = new gTexture[material.m_textureCount];
        stream->readS(material.m_textures, material.m_textureCount);
    }

    grm::u32 nSize;
    stream->readS(&nSize, 1);

    Transform *transform = nullptr;
    std::condition_variable cond;

    Event::Add(&AssetsManager::SetupGameObject, std::ref(cond), std::ref(handle), std::ref(transform), pModel, m_mesh, m_materials, parent);

    std::unique_lock<std::mutex> lock(s_gameObjectMutex);
    cond.wait(lock, [&transform] {
        return transform;
    });
    lock.unlock();

    for (auto &material : m_materials) {
        delete[] material.m_textures;
    }


    for (int i=0;i<nSize;++i) {
        ReadChildren(transform, handle, stream);
    }
    
    // lock.lock();
    transform = nullptr;
}

void AssetsManager::doInBackgroundGameObject(GameObjectHandle handle) {
    Stream stream(handle.m_path, std::ios_base::in | std::ios_base::binary);
    if (!stream.is_open()) {
        return Console::log(LogInfo::ERROR, "Failed to load model. " + handle.m_path);
    }

    gHeader pHeader;
    if (!stream.readS(&pHeader, 1)) {
        return Console::log(LogInfo::ERROR, "Failed to read header to file.");
    }

    if (std::string(pHeader.signature) != "grmd") {
        return Console::log(LogInfo::ERROR, "Model invalid: " + handle.m_path);
    }

    ReadChildren(nullptr, handle, &stream);

    if (handle.m_function && handle.m_gameObject)
        Event::Add(handle.m_function, handle.m_gameObject);
}

void AssetsManager::ProcessBinary() {
    std::unique_lock<std::mutex> lock(s_gameObjectMutex);
    while (!s_gameObjectFromLoader.empty()) {
        auto handle = s_gameObjectFromLoader.front();
        s_gameObjectFromLoader.pop_front();
        lock.unlock();

        doInBackgroundGameObject(handle);

        lock.lock();
    }
}

void AssetsManager::SetupTextureMaterial(Material *material, int id, grr::gTexture * texture) {
    if (!material) {
        return;
    }
    material->setTexture(id, texture);
}

void AssetsManager::SetGameObjectList(const std::string &path, std::function<void(GameObject *)> func) {
    if (!File::Exists(path)) {
        return LOG_ERROR("[ AssetsManager::GameObject ] File not found: " + path);
    }

    {
        std::lock_guard<std::mutex> lock(s_gameObjectMutex);
        s_gameObjectFromLoader.emplace_back(func, path);
    }

    Threadpool::AddTask(&AssetsManager::ProcessBinary);
}

// TODO: aicionar lista de callbacks a serem chamadas apos shader serem adicionado a thread
void AssetsManager::SetShaderList(const std::string &name, const std::string &frag, const std::string &vert, std::function<void(grr::gShader *)> func) {
    if (!File::Exists(frag)) {
        return LOG_ERROR("[ AssetsManager-Shader ] File do not exists: " + frag);
    }

    if (!File::Exists(vert)) {
        return LOG_ERROR("[ AssetsManager-Shader ] File do not exists: " + vert);
    }

    auto it = s_shaderList.find(name);
    if (it != s_shaderList.end()) {
        if (func)
            func(&(*it->second));
        return;
    }

    {
        std::lock_guard<std::mutex> lock(s_shaderMutex);
        auto it = std::find_if(s_shaderFromLoader.begin(), s_shaderFromLoader.end(), [name](auto &handle) {
            return handle.m_name == name;
        });
        
        if (it != s_shaderFromLoader.end()) {
            it->add(func);
            return;
        }
        s_shaderFromLoader.emplace_back(func, name, frag, vert);
    }

    Threadpool::AddTask(&AssetsManager::ProcessShader);
}

void AssetsManager::SetTextureList(const std::string &path, std::function<void(grr::gTexture *)> func) {
    if (!File::Exists(path)) {
        return LOG_ERROR("[ AssetsManager::Texture ] File not found: " + path);
    }

    {
        std::lock_guard<std::mutex> lock(m_mutexTexture);
        auto map = m_textureMap.find(path);
        if (map != m_textureMap.end()) {
            if (func) {
                func(&(*map->second));
            }
            return;
        }

        auto it = m_mapHandleTexture.find(path);
        if (it != m_mapHandleTexture.end()) {
            it->second->add(func);
            return;
        }
        auto &handle = m_textureHandle.emplace_back(func, path);
        m_mapHandleTexture.emplace(path, &handle);
    }

    Threadpool::AddTask(&AssetsManager::ProcessTexture);
}

grm::size AssetsManager::GetTextureCount() {
    return m_textureMap.size();
}

grm::size AssetsManager::GetShaderCount() {
    return s_shaderList.size();
}

grm::size AssetsManager::GetMeshCount() {
    auto count = 0;
    {
        std::lock_guard<std::mutex> lock(m_meshMutex);
        count = s_meshList.size();
    }
    return count;
}

std::string AssetsManager::GetTextureKeyMap(grr::gTexture *texture) {
    auto it = std::find_if(m_textureMap.begin(), m_textureMap.end(), [&texture](const auto &tuple) {
        return &(*std::get<std::unique_ptr<grr::gTexture>>(tuple)) == texture;
    });

    if (it == m_textureMap.end()) {
        return "";
    }
    return it->first;
}

std::unordered_map<std::string, std::unique_ptr<grr::gTexture>> &AssetsManager::GetTextureMap() {
    return m_textureMap;
}

TextureHandle::TextureHandle(std::function<void(grr::gTexture *)> function, std::string path) : m_path(path) {
    if (!function) {
        return;
    }
    m_functions.push_back(function);
}

void TextureHandle::invoke(grr::gTexture *texture) {
    for (auto it = m_functions.begin(); it != m_functions.end();) {
        if (*it) {
            (*it)(texture);
        }
        it = m_functions.erase(it);
    }
}

void TextureHandle::add(std::function<void(grr::gTexture *)> function) {
    m_functions.push_back(function);
}

ShaderHandle::ShaderHandle(std::function<void(grr::gShader *)> function, std::string name, std::string frag, std::string vert) : m_name(name), m_frag(frag), m_vert(vert) {
    if (!function) {
        return;
    }
    m_functions.push_back(function);
}

void ShaderHandle::invoke(grr::gShader *shader) {
    for (auto it = m_functions.begin(); it != m_functions.end();) {
        if (*it) {
            (*it)(shader);
        }
        it = m_functions.erase(it);
    }
}

void ShaderHandle::add(std::function<void(grr::gShader *)> function) {
    m_functions.push_back(function);
}
