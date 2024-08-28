#include "resources.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../core/stb/stb_image.h"

#include "filesystem/filesystem.h"
#include "filesystem/stream.h"

#include "core/animation/animation.hpp"

#include "core/scene.hpp"

#include "../ecs/gameObject.h"

#include "script/meshFilter.hpp"
#include "script/meshRender.hpp"
#include "script/transform.hpp"
#include "script/animator.hpp"

#include "console.hpp"
#include "settings.h"

#include "event.hpp"
#include "timer.h"

#include <filesystem>
#include <cassert>
#include <future>

grm::u32 Resources::m_processCount = 0;

/***************** Shader *****************/
std::unordered_map<std::string, std::unique_ptr<grr::gShader>> Resources::s_shaderList;
std::list<std::tuple<std::tuple<std::string, std::string>, grr::gShader *>> Resources::s_shaderFromLoader;
std::mutex Resources::s_shaderMutex;

std::vector<std::function<bool(grr::gShader *, const std::string &)>> Resources::m_functions;

/***************** Texture *****************/
std::unordered_map<std::string, std::unique_ptr<grr::gTexture>> Resources::s_textureList;
std::list<std::tuple<std::string, grr::gTexture *>> Resources::s_textureFromLoader;
std::mutex Resources::s_textureMutex;

/***************** Animation *****************/
std::unordered_map<std::string, std::unique_ptr<Animation>> Resources::s_animationList;
std::list<std::tuple<std::string, Animation *>> Resources::s_animationFromLoader;
std::mutex Resources::s_animatioMutex;

/***************** GameObject *****************/
std::list<std::tuple<std::string, Transform *>> Resources::s_gameObjectFromLoader;
std::mutex Resources::s_gameObjectMutex;

/***************** Mesh *****************/
std::unordered_map<std::string, grm::size> Resources::s_meshMapID;
std::unordered_map<std::string, Mesh *> Resources::s_meshMapName;
std::vector<std::unique_ptr<Mesh>> Resources::s_meshList;
std::mutex Resources::m_meshMutex;

std::thread Resources::m_thread;

void Resources::Initialize() {
    stbi_set_flip_vertically_on_load(true);

    m_thread = std::thread(&Resources::OnThread);
}

void Resources::Shutdown() {
    if (m_thread.joinable()) {
        m_thread.join();
    }
}

/***************** TEXTURE ****************/
grr::gTexture *Resources::LoadTexture(const std::string &path) {
    if (!std::filesystem::exists(path)) { 
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

    auto m_texture = new grr::gTexture();
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

/***************** SHADER ****************/
grr::gShader *Resources::LoadShader(const std::string &frag, const std::string &vert) {
    if (!Filesystem::Exists(frag)) {
        Console::log(LogInfo::ERROR, "File do not found: " + frag);
        return nullptr;
    }

    if (!Filesystem::Exists(vert)) {
        Console::log(LogInfo::ERROR, "File do not found: " + vert);
        return nullptr;
    }

    auto name = frag + vert;

    auto it = s_shaderList.find(name);
    if (it != s_shaderList.end()) {
        return &(*(*it).second);
    }

    {
        std::lock_guard<std::mutex> lock(s_shaderMutex);
        auto it = std::find_if(s_shaderFromLoader.begin(), s_shaderFromLoader.end(), [&name](auto &tuple) {
            auto &&[frag, vert] = std::get<std::tuple<std::string, std::string>>(tuple);

            return (frag + vert) == name;
        });

        if (it != s_shaderFromLoader.end()) {
            return std::get<grr::gShader *>((*it));
        }
    }

    grr::gShader *shader = new grr::gShader();

    Console::log(LogInfo::INFO, "Processing Shader: "+ frag + " " + vert);

    {
        std::lock_guard<std::mutex> lock(s_shaderMutex);
        s_shaderFromLoader.emplace_back(std::make_tuple(frag, vert), shader);
    }
    return shader;
}

GameObject *Resources::LoadGameObject(const std::string &path) {
    Console::log(LogInfo::INFO, "Reading model...");

    auto name = Filesystem::GetFileName(path);

    auto gameObject = Scene::Create(name);
    auto &transform = gameObject->getTransform();

    {
        std::lock_guard<std::mutex> lock(s_gameObjectMutex);
        s_gameObjectFromLoader.emplace_back(path, &transform);
    }

    return gameObject;
}

Animation *Resources::LoadAnimation(const std::string &path) {
    if (!Filesystem::Exists(path)) {
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

void Resources::AddShaderLoaded(std::function<bool(grr::gShader *, const std::string &)> func) {
    m_functions.push_back(func);
}

Mesh *Resources::FindMesh(const std::string &name) {
    auto it = s_meshMapName.find(name);
    if (it == s_meshMapName.end()) {
        return nullptr;
    }
    return it->second;
}

Mesh *Resources::AddMesh(const std::string &name, std::unique_ptr<Mesh> mesh) {
    s_meshMapID.emplace(name, s_meshList.size());

    auto meshPtr = &(*s_meshList.emplace_back(std::move(mesh)));

    s_meshMapName.emplace(name, meshPtr);

    return meshPtr;
}

void Resources::DeleteMesh(Mesh *target) {
    if (!target) {
        return;
    }
    
    // std::unique_lock<std::mutex> lock(m_meshMutex);
    // auto it = s_meshList.find(target->getName());
    // if (it == s_meshList.end()) {
    //     return;
    // }
    // s_meshList.erase(it);
    // lock.unlock();
}

/***************** Texture Process ****************/
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

    grr::gTexture::UpdateTexture(texture, width, height, grr::gTextureFlags_MipMaps | grr::gTextureFlags_Filter_Bilinear, format, pixels);

    stbi_image_free(pixels);

    Console::log(LogInfo::INFO, "Texture loaded: " + path);
}

void Resources::ProcessTexture() {
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

        Event::Add(Resources::SetupTexture, texture, width, height, ch, (void *)pixels, path);

        lock.lock();
        s_textureFromLoader.pop_front();
    }
}

/***************** Shader Process ****************/
void Resources::SetupShader(grr::gShader *shader, const std::string &frag, const std::string &vert, const std::string &path) {
    Console::log(LogInfo::WARNING, "Shader loadedd sucessfull.");

    const char *fShader = frag.c_str();
    const char *vShader = vert.c_str();

    grr::gShader::Bind(shader);
    grr::gShader::SetupShader(&fShader, &vShader);
    grr::gShader::Unbind();

    if (CheckShaderError(shader)) {
        delete shader;
        return;
    }

    m_functions.erase(std::remove_if(m_functions.begin(), m_functions.end(), [&shader, &path](auto &func) {
        return func(shader, path);
    }), m_functions.end());

        s_shaderList.emplace(path, std::unique_ptr<grr::gShader>(shader));
}

bool Resources::CheckShaderError(grr::gShader *shader) {
    if (shader->isValid()) {
        return false;
    }

    std::stringstream iss(grr::gError::GetError());

    std::string line;
    while (std::getline(iss, line)) {
        Console::log(LogInfo::ERROR, line);
    }
    grr::gError::Clean();

    return true;
}

void Resources::ProcessShader() {
    std::unique_lock<std::mutex> lock(s_shaderMutex);
    while (!s_shaderFromLoader.empty()) {
        auto &&[tuple, shader] = s_shaderFromLoader.front();

        auto &&[frag, vert] = tuple;

        lock.unlock();

        // Processing shader

        // Loading Fragment shader
        Console::log(LogInfo::WARNING, "Loading FragmentShader: " + frag + " - " + vert);

        auto fragSource = Filesystem::GetFileContent(frag);
        if (fragSource.empty()) {
            Console::log(LogInfo::ERROR, "Falied to load: " + frag);
            delete shader;

            lock.lock();
            s_shaderFromLoader.pop_front();
            continue;
        }

        // Loading vertex shader
        Console::log(LogInfo::WARNING, "Loading VertexShader: " + vert);
        auto vertSource = Filesystem::GetFileContent(vert);
        if (vertSource.empty()) {
            Console::log(LogInfo::ERROR, "Falied to load: " + vert);
            delete shader;

            lock.lock();
            s_shaderFromLoader.pop_front();
            continue;
        }

        Event::Add(Resources::SetupShader, shader, fragSource, vertSource, frag + vert);

        lock.lock();
        s_shaderFromLoader.pop_front();
    }
}

void Resources::SetupAnimation(const std::string &path, FbxAnimation *temp, Animation *animation) {
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

void Resources::doAnimationInBackground(const std::string &path, Animation *target) {
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

    Event::Add(&Resources::SetupAnimation, path, fbxAnimation, target);
}

void Resources::ProcessAnimation() {
    std::unique_lock<std::mutex> lock(s_animatioMutex);
    while (!s_animationFromLoader.empty()) {
        auto &&[path, animation] = s_animationFromLoader.front();
        lock.unlock();

        doAnimationInBackground(path, animation);

        lock.lock();
        s_animationFromLoader.pop_front();
    }
}

void Resources::ReadNode(FbxNode &parent) {
    gModel pModel;
    if (!Stream::read(&pModel, 1)) {
        return Console::log(LogInfo::ERROR, "Falied to read model");
    }

    grm::u32 meshCount;
    if (!Stream::read(&meshCount, 1)) {
        return Console::log(LogInfo::ERROR, "Failed to read mesh count to file.");
    }

    std::vector<gMesh> meshes(meshCount);

    for (auto &mesh : meshes) {
        // read name
        if (!Stream::read<char>(mesh.m_name, 256)) {
            continue;
        }

        // bone
        if (!Stream::read<bool>(&mesh.m_hasBone, 1)) {
            continue;
        }

        // read vertices
        if (!Stream::read(&mesh.m_verticeCount, 1)) {
            continue;
        }

        mesh.m_vertices = new gVertex[mesh.m_verticeCount];
        if (!Stream::read(mesh.m_vertices, mesh.m_verticeCount)) {
            continue;
        }

        // read indices
        if (!Stream::read(&mesh.m_indicesCount, 1)) {
            continue;
        }

        mesh.m_indices = new grm::u32[mesh.m_indicesCount];
        if (!Stream::read(mesh.m_indices, mesh.m_indicesCount)) {
            continue;
        }
    
        // bad alloc
        // if (!mesh.m_hasBone) {
        //     continue;
        // }

        // read bones
        if (!Stream::read(&mesh.m_boneCount, 1)) {
            continue;
        }

        mesh.m_bones = new gBone[mesh.m_boneCount];
        if (!Stream::read(mesh.m_bones, mesh.m_boneCount)) {
            continue;
        }
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

            delete[] pMesh.m_vertices;
            delete[] pMesh.m_indices;

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

            delete[] pMesh.m_bones;
        }

        m_mesh = Resources::FindMesh(meshName);
        if (!m_mesh) {
            mesh.setName(meshName);

            m_mesh = Resources::AddMesh(meshName, std::make_unique<Mesh>(mesh));
        }
    }
    
    grm::u32 mSize;
    if (!Stream::read(&mSize, 1)) {
        return;
    }

    std::vector<gMaterial> m_materials;
    if (mSize >= 1) {
        m_materials.resize(mSize);
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
    }

    grm::u32 nSize;
    Stream::read(&nSize, 1);

    auto &node = parent.addNode(pModel.m_name, &parent);
    node.setScale(pModel.m_scale);
    node.setRotation(pModel.m_rotation);
    node.setPosition(pModel.m_position);

    node.addMesh(s_meshMapID[m_mesh->getName()]);

    for (int i=0;i<nSize;++i) {
        ReadNode(node);
    }
}

void Resources::SetupHierarchi(const FbxNode &node, Transform *parent) {
    auto& gameObject = GameObject::Create(node.getName(), "unttaged");

    auto &transform = gameObject.getTransform();
    transform.setParent(parent);

    transform.setLocalScale(node.getScale());
    transform.setLocalRotation(node.getRotation());
    transform.setLocalPosition(node.getPosition());

    if (node.hasMeshe()) {
        auto& meshFilter = gameObject.addComponent<MeshFilter>();
        for (auto &meshID : node.getMeshes()) {
            auto &mesh = s_meshList[meshID];

            meshFilter.setMesh(&(*mesh));
        }

        auto &meshRender = gameObject.addComponent<MeshRender>();

        meshRender.addMaterial(0, Material{"Def"});

        meshRender.build();
    }

    for (auto &child : node.getChildren()) {
        SetupHierarchi(child, &transform);
    }
}

void Resources::doInBackgroundGameObject(const std::string &path, Transform *target) {
    if (!Stream::open(path, std::ios_base::in | std::ios_base::binary)) {
        return Console::log(LogInfo::ERROR, "Falied open binary: " + path);
    }
    
    gHeader pHeader;
    if (!Stream::read(&pHeader, 1)) {
        return Console::log(LogInfo::ERROR, "Failed to read header to file.");
    }

    FbxNode root;

    ReadNode(root);

    Stream::close();

    for (auto &child : root.getChildren()) {
        Event::Add(&Resources::SetupHierarchi, child, target);
        // PrintHierarchi(child, target);
    }
}

void Resources::ProcessGameObject() {
    std::unique_lock<std::mutex> lock(s_gameObjectMutex);
    while (!s_gameObjectFromLoader.empty()) {
        auto &&[path, transform] = s_gameObjectFromLoader.front();
        lock.unlock();

        doInBackgroundGameObject(path, transform);

        lock.lock();
        s_gameObjectFromLoader.pop_front();
    }
}

void Resources::OnThread() {
    TimeLast timer(5.0f);
    
    while (Settings::IsEngineRunning()) {
        timer.SleepTick();
        
        ProcessShader();

        ProcessTexture();

        ProcessAnimation();

        ProcessGameObject();
    }
}

const grm::size Resources::GetTextureCount() {
    return s_textureList.size();
}

const grm::size Resources::GetShaderCount() {
    return s_shaderList.size();
}

const grm::size Resources::GetMeshCount() {
    auto count = 0;
    {
        std::lock_guard<std::mutex> lock(m_meshMutex);
        count = s_meshList.size();
    }
    return count;
}




