#include "assetsManager.hpp"

#include "core/file/stream.hpp"
#include "core/file/file.hpp"

#include "core/animation/animation.hpp"

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

#define STB_IMAGE_IMPLEMENTATION
#include "../core/stb/stb_image.h"

grm::u32 AssetsManager::m_processCount = 0;

/***************** Shader *****************/
std::unordered_map<std::string, std::unique_ptr<grr::gShader>> AssetsManager::s_shaderList;
std::list<ShaderHandle> AssetsManager::s_shaderFromLoader;
std::mutex AssetsManager::s_shaderMutex;

std::vector<std::function<bool(void *, const std::string &)>> AssetsManager::m_functions;

/***************** Texture *****************/
std::unordered_map<std::string, std::unique_ptr<grr::gTexture>> AssetsManager::s_textureList;
std::list<std::tuple<std::string, grr::gTexture *>> AssetsManager::s_textureFromLoader;
std::mutex AssetsManager::s_textureMutex;

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
std::unordered_map<std::string, std::vector<Material>> AssetsManager::m_materialMap;

std::thread AssetsManager::m_thread;

void AssetsManager::Initialize() {
    stbi_set_flip_vertically_on_load(true);

    m_thread = std::thread(&AssetsManager::OnThread);
}

void AssetsManager::Shutdown() {
    if (m_thread.joinable()) {
        m_thread.join();
    }

    // clear assets
    m_processCount = 0;

    /***************** Shader *****************/
    s_shaderList.clear();
    s_shaderFromLoader.clear();

    /***************** Functions *****************/
    m_functions.clear();

    /***************** Texture *****************/
    s_textureList.clear();
    s_textureFromLoader.clear();

    /***************** Animation *****************/
    s_animationList.clear();
    s_animationFromLoader.clear();

    /***************** GameObject *****************/
    s_gameObjectFromLoader.clear();

    /***************** Mesh *****************/
    s_meshList.clear();

    /***************** Material *****************/
    m_materialMap.clear();
}

/***************** TEXTURE ****************/
grr::gTexture *AssetsManager::LoadTexture(const std::string &path) {
    if (!File::Exists(path)) { 
        Console::log(LogInfo::ERROR, "[ Resources::LoadTexture ] Falied to load texture: " + path);
        return nullptr;
    }

    auto it = s_textureList.find(path);
    if (it != s_textureList.end()) {
        Console::log(LogInfo::WARNING, "The texture has already been loaded.");
        return &(*(*it).second);
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

    auto m_texture = grr::gTexture::Create();
    if (!m_texture) {
        Console::log(LogInfo::ERROR, "[ Resources::LoadTexture ] Falied to create texture.");
        return nullptr;
    }
    
    {
        std::lock_guard<std::mutex> lock(s_textureMutex);
        s_textureFromLoader.emplace_back(path, m_texture);
    }
    return m_texture;
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

void AssetsManager::AddPostLoader(std::function<bool(void *, const std::string &)> func) {
    m_functions.push_back(func);
}

grr::gShader *AssetsManager::FindShader(const std::string &name) {
    auto it = s_shaderList.find(name);
    if (it == s_shaderList.end()) {
        return nullptr;
    }
    return &(*it->second);
}

Mesh *AssetsManager::FindMesh(const std::string &name) {
    std::unique_lock<std::mutex> lock(m_meshMutex);
    auto it = s_meshList.find(name);
    if (it == s_meshList.end()) {
        return nullptr;
    }
    auto mesh = &(*(*it).second);
    lock.unlock();
    return mesh;
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

std::vector<Material> *AssetsManager::FindMaterial(const std::string &name) {
    auto it = m_materialMap.find(name);
    if (it == m_materialMap.end()) {
        return nullptr;
    }
    return &it->second;
}

/***************** Texture Process ****************/
void AssetsManager::SetupTexture(grr::gTexture *texture, int width, int height, int ch, void *pixels, std::string path) {
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

    grr::gTexture::Bind(texture);
    grr::gTexture::UpdateTexture(width, height, grr::gTextureFlags_MipMaps | grr::gTextureFlags_Filter_Bilinear, format, pixels);

    stbi_image_free(pixels);

    m_functions.erase(std::remove_if(m_functions.begin(), m_functions.end(), [&texture, &path](auto &func) {
        return func(texture, path);
    }), m_functions.end());

    Console::log(LogInfo::INFO, "Texture loaded: " + path);
}

void AssetsManager::ProcessTexture() {
    std::unique_lock<std::mutex> lock(s_textureMutex);
    while (!s_textureFromLoader.empty()) {
        auto [path, texture] = s_textureFromLoader.front();
        lock.unlock();

        int width, height, ch;
        auto pixels = stbi_load(path.c_str(), &width, &height, &ch, 0);
        if (!pixels) {
            Console::log(LogInfo::ERROR, "Failed to load texture: " + path);

            delete texture;

            lock.lock();
            s_textureFromLoader.pop_front();
            continue;
        }

        Event::Add(AssetsManager::SetupTexture, texture, width, height, ch, (void *)pixels, path);

        lock.lock();
        s_textureFromLoader.pop_front();
    }
}

/***************** Shader Process ****************/
void AssetsManager::SetupShader(ShaderHandle handle, const std::string &frag, const std::string &vert) {
    std::function<void(grr::gShader *)> func = nullptr;
    std::string name;
    {
        std::lock_guard<std::mutex> lock(s_shaderMutex);
        func = handle.m_function;
        name = handle.m_name;
    }

    Console::log(LogInfo::INFO, "Shader loadedd sucessfull: " + name);

    const char *fShader = frag.c_str();
    const char *vShader = vert.c_str();

    auto shader = grr::gShader::Create(&fShader, &vShader);

    if (CheckShaderError(shader, name)) {
        delete shader;
        return;
    }

    if (func) {
        func(shader);
    }

    s_shaderList.emplace(name, std::unique_ptr<grr::gShader>(shader));
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
        // lock.unlock();

        // Processing shader
        auto fragSource = File::GetFileContent(handle.m_frag);
        if (fragSource.empty()) {
            Console::log(LogInfo::ERROR, "Falied to shader: " + handle.m_name + " Fragment: " + handle.m_frag);

            // lock.lock();
            continue;
        }

        // Loading vertex shader
        auto vertSource = File::GetFileContent(handle.m_vert);
        if (vertSource.empty()) {
            Console::log(LogInfo::ERROR, "Falied to shader: " + handle.m_name + " Vertex: " + handle.m_vert);

            // lock.lock();
            continue;
        }

        Event::Add(AssetsManager::SetupShader, handle, fragSource, vertSource);

        // lock.lock();
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
    if (!Stream::open(path, std::ios_base::in | std::ios_base::binary)) {
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

std::condition_variable cond;
void AssetsManager::SetupGameObject(GameObjectHandle &handle, Transform *&out, gModel &model, Mesh *mesh, std::vector<gMaterial> &materials, Transform *parent) {
    // transformaçoes
    GameObject *gameObject = nullptr;
    {
        std::lock_guard<std::mutex> lock(s_gameObjectMutex);
        gameObject = (handle.m_gameObject != nullptr ? &GameObject::Create(model.m_name, "unttaged") : &Scene::GetCurrentScene()->CreateGameObject(model.m_name, "unttaged"));
        if (!handle.m_gameObject) {
            handle.m_gameObject = gameObject;
        }
    }

    auto &transform = gameObject->getTransform();

    transform.setLocalScale(model.m_scale);
    transform.setLocalRotation(model.m_rotation);
    transform.setLocalPosition(model.m_position);

    if (parent)
        transform.setParent(parent);

    if (mesh) {
        auto &meshFilter = gameObject->addComponent<MeshFilter>();
        meshFilter.setMesh(mesh);

        auto &meshRender = gameObject->addComponent<MeshRender>();

        for (auto &material : materials) {
            auto &m = meshRender.addMaterial(material.m_name, material.m_color);

            for (int i=0;i<material.m_textureCount;++i) { 
                auto& texture = material.m_textures[i];

                m.setTexture(i, AssetsManager::LoadTexture(texture.m_path));
            }

            delete[] material.m_textures;
        }

        meshRender.build();
    }

    {
        std::lock_guard<std::mutex> lock(s_gameObjectMutex);
        out = &transform;
    }
    cond.notify_one();
}

void AssetsManager::doInBackgroundGameObject(GameObjectHandle &handle) {
    if (!Stream::open(handle.m_path, std::ios_base::in | std::ios_base::binary)) {
        return Console::log(LogInfo::ERROR, "Failed to load model. " + handle.m_path);
    }

    gHeader pHeader;
    if (!Stream::read(&pHeader, 1)) {
        return Console::log(LogInfo::ERROR, "Failed to read header to file.");
    }

    if (std::string(pHeader.signature) != "grmd") {
        return Console::log(LogInfo::ERROR, "Model invalid: " + handle.m_path);
    }

    ReadChildren(nullptr, handle);

    Stream::close();


    std::function<void(GameObject *)> func = nullptr;
    GameObject *gameObject = nullptr;

    {
        std::lock_guard<std::mutex> lock(s_gameObjectMutex);
        gameObject = handle.m_gameObject;
        func = handle.m_function;
    }

    if (func && gameObject)
        Event::Add(func, gameObject);
}

void AssetsManager::ProcessBinary() {
    std::unique_lock<std::mutex> lock(s_gameObjectMutex);
    while (!s_gameObjectFromLoader.empty()) {
        auto &handle = s_gameObjectFromLoader.front();
        lock.unlock();

        doInBackgroundGameObject(handle);

        lock.lock();
        s_gameObjectFromLoader.pop_front();
    }
}

std::unordered_map<std::string, std::vector<Material>> &AssetsManager::GetMaterialMap() {
    return m_materialMap;
}

#include "timer.h"

void AssetsManager::OnThread() {
    TimeLast last(5.0f);
    while (Settings::IsEngineRunning()) {
        last.SleepTick();

        ProcessShader();

        ProcessTexture();
    
        ProcessAnimation();
    
        ProcessBinary();
    }
}

void AssetsManager::ReadChildren(Transform *parent, GameObjectHandle &handle) {
    gModel pModel;
    if (!Stream::read(&pModel, 1)) {
        return;
    }

    grm::u32 meshCount;
    if (!Stream::read(&meshCount, 1)) {
        Console::log(LogInfo::ERROR, "Failed to read mesh count to file.");
        return;
    }

    std::vector<gMesh> meshes(meshCount);
    for (auto &mesh : meshes) {
        mesh.deserialize();
    }

    Animator *m_animator = nullptr;
    Mesh *m_mesh = nullptr;

    // adiciona o mesh componenet
    if (!meshes.empty()) {
        Mesh mesh; //meshFilter.getMesh();

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

        m_mesh = AssetsManager::FindMesh(meshName);
        if (!m_mesh) {
            mesh.setName(meshName);

            m_mesh = AssetsManager::AddMesh(meshName, mesh);
        }
    }

    grm::u32 mSize;
    if (!Stream::read(&mSize, 1)) {
        return;
    }

    std::vector<gMaterial> m_materials(mSize);
    for (auto &material : m_materials) {
        Stream::read(material.m_name, 256);
        Stream::read(&material.m_color, 1);
        Stream::read(&material.m_roughness, 1);
        Stream::read(&material.m_metallic, 1);
        Stream::read(&material.m_ambientOcclusion, 1);

        Stream::read(&material.m_textureCount, 1);

        material.m_textures = new gTexture[material.m_textureCount];
        Stream::read(material.m_textures, material.m_textureCount);

        for (int i=0;i<material.m_textureCount;++i) {
            auto &texture = material.m_textures[i];
        }
    }

    grm::u32 nSize;
    Stream::read(&nSize, 1);

    Transform *transform = nullptr;

    Event::Add(&AssetsManager::SetupGameObject, std::ref(handle), std::ref(transform), pModel, m_mesh, m_materials, parent);

    std::unique_lock<std::mutex> lock(s_gameObjectMutex);
    cond.wait(lock, [&transform] {
        return transform;
    });
    lock.unlock();

    for (int i=0;i<nSize;++i) {
        ReadChildren(transform, handle);
    }
    
    // lock.lock();
    transform = nullptr;
}

void AssetsManager::SetGameObjectList(const std::string &path, std::function<void(GameObject *)> &func) {
    if (!File::Exists(path)) {
        Console::log(LogInfo::ERROR, "Binary do not found: " + path);
        return;
    }

    {
        std::lock_guard<std::mutex> lock(s_gameObjectMutex);
        s_gameObjectFromLoader.emplace_back(func, path);
    }
}

void AssetsManager::SetShaderList(const std::string &name, const std::string &frag, const std::string &vert, std::function<void(grr::gShader *)> &func) {
    if (!File::Exists(frag)) {
        return LOG_ERROR("[ AssetsManager-Shader ] File do not exists: " + frag);
    }

    if (!File::Exists(vert)) {
        return LOG_ERROR("[ AssetsManager-Shader ] File do not exists: " + vert);
    }

    {
        std::lock_guard<std::mutex> lock(s_shaderMutex);
        s_shaderFromLoader.emplace_back(func, name, frag, vert);
    }
}

grm::size AssetsManager::GetMaterialCount() {
    return m_materialMap.size();
}

grm::size AssetsManager::GetTextureCount() {
    return s_textureList.size();
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
    auto it = std::find_if(s_textureList.begin(), s_textureList.end(), [&texture](const auto &tuple) {
        return &(*std::get<std::unique_ptr<grr::gTexture>>(tuple)) == texture;
    });

    if (it == s_textureList.end()) {
        return "";
    }
    return it->first;
}

std::unordered_map<std::string, std::unique_ptr<grr::gTexture>> &AssetsManager::GetTextureMap() {
    return s_textureList;
}
