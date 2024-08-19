#include "resources.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../core/stb/stb_image.h"

#include "../filesystem/filesystem.h"

#include "../core/binary/gHeader.hpp"

#include "../core/scene.hpp"

#include "../ecs/gameObject.h"

#include "script/meshFilter.hpp"
#include "script/meshRender.hpp"
#include "script/transform.hpp"
#include "script/animator.hpp"

#include "console.hpp"
#include "settings.h"

#include "event.hpp"

#include <filesystem>
#include <future>

u32 Resources::m_processCount = 0;

/***************** Shader *****************/
std::unordered_map<std::string, std::unique_ptr<grr::gShader>> Resources::s_shaderList;
std::list<std::tuple<std::string, grr::gShader *>> Resources::s_shaderFromLoader;
std::mutex Resources::s_shaderMutex;

std::vector<std::function<bool(grr::gShader *, const std::string &)>> Resources::m_functions;

/***************** Texture *****************/
std::unordered_map<std::string, std::unique_ptr<grr::gTexture>> Resources::s_textureList;
std::list<std::tuple<std::string, grr::gTexture *>> Resources::s_textureFromLoader;
std::mutex Resources::s_textureMutex;

/***************** Mesh *****************/
std::unordered_map<std::string, std::unique_ptr<Mesh>> Resources::s_meshList;

void Resources::Initialize() {
    stbi_set_flip_vertically_on_load(true);
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
grr::gShader *Resources::LoadShader(const std::string &path) {
    if (!Filesystem::Exists(path + ".frag")) {
        Console::log(LogInfo::ERROR, "File do not found: " + path + ".frag");
        return nullptr;
    }

    if (!Filesystem::Exists(path + ".vert")) {
        Console::log(LogInfo::ERROR, "File do not found: " + path + ".vert");
        return nullptr;
    }

    auto it = s_shaderList.find(path);
    if (it != s_shaderList.end()) {
        return (*it).second.get();
    }

    grr::gShader *shader = new grr::gShader();
    if (!shader) {
        Console::log(LogInfo::ERROR, "Falied to create shader.");
        return nullptr;
    }

    Console::log(LogInfo::INFO, "Processing Shader: "+ path);

    {
        std::lock_guard<std::mutex> lock(s_shaderMutex);
        s_shaderFromLoader.emplace_back(path, shader);
    }
    return shader;
}

std::string FindNewName(std::string name, int id = 0) {
    if (GameObject::Find(name)) {
        name += std::to_string(id);
        return FindNewName(name, id++);
    }
    return name;
}

GameObject *ReadChildren(std::istream &is) {
    gModel pModel;
    if (!(is.read(reinterpret_cast<char *>(&pModel), sizeof(gModel)))) {
        return nullptr;
    }

    grm::u32 meshCount;
    if (!is.read(reinterpret_cast<char *>(&meshCount), sizeof(grm::u32))) {
        Console::log(LogInfo::ERROR, "Failed to read mesh count to file.");
        return nullptr;
    }

    std::vector<gMesh> meshes(meshCount);

    for (auto &mesh : meshes) {
        // read name
        if (!is.read(reinterpret_cast<char *>(mesh.m_name), sizeof(char) * 256)) {
            continue;
        }

        // bone
        if (!is.read(reinterpret_cast<char *>(&mesh.m_hasBone), sizeof(bool))) {
            continue;
        }

        // read vertices
        if (!is.read(reinterpret_cast<char *>(&mesh.m_verticeCount), sizeof(grm::u32))) {
            continue;
        }

        mesh.m_vertices = new gVertex[mesh.m_verticeCount];
        if (!is.read(reinterpret_cast<char *>(mesh.m_vertices), mesh.m_verticeCount * sizeof(gVertex))) {
            continue;
        }

        // read indices
        if (!is.read(reinterpret_cast<char *>(&mesh.m_indicesCount), sizeof(grm::u32))) {
            continue;
        }

        mesh.m_indices = new grm::u32[mesh.m_indicesCount];
        if (!is.read(reinterpret_cast<char *>(mesh.m_indices), mesh.m_indicesCount * sizeof(grm::u32))) {
            continue;
        }
    
        // bad alloc
        // if (!mesh.m_hasBone) {
        //     continue;
        // }

        // read bones
        if (!is.read(reinterpret_cast<char *>(&mesh.m_boneCount), sizeof(grm::u32))) {
            continue;
        }

        mesh.m_bones = new gBone[mesh.m_boneCount];
        if (!is.read(reinterpret_cast<char *>(mesh.m_bones), mesh.m_boneCount * sizeof(gBone))) {
            continue;
        }
    }

    // Cria um game object 
    auto gameObject = &GameObject::Create(pModel.m_name, "tag");

    // transformaçoes 
    auto &transform = gameObject->getTransform();

    transform.setLocalScale(pModel.m_scale);
    transform.setLocalRotation(pModel.m_rotation);
    transform.setLocalPosition(pModel.m_position);

    Animator *m_animator = nullptr;

    // adiciona o mesh componenet
    if (!meshes.empty()) {
        auto &meshFilter = gameObject->addComponent<MeshFilter>();
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
                m_animator = &gameObject->addComponent<Animator>();
            }

            for (auto i=0;i<pMesh.m_boneCount;++i) {
                auto &pBone = pMesh.m_bones[i];

                m_animator->addBone(pBone.m_name, pBone.m_position, pBone.m_scale, pBone.m_rotation, pBone.m_id);
            }

            delete[] pMesh.m_bones;
        }

        auto m_mesh = Resources::FindMesh(meshName);
        if (m_mesh) {
            meshFilter.setMesh(m_mesh);
        } else {
            mesh.setName(meshName);

            meshFilter.setMesh(Resources::AddMesh(meshName, std::make_unique<Mesh>(mesh)));
        }
    }

    grm::u32 mSize;
    if (!is.read(reinterpret_cast<char *>(&mSize), sizeof(grm::u32))) {
        return nullptr;
    }

    if (mSize >= 1) {
        std::vector<gMaterial> m_materials(mSize);
        for (auto &material : m_materials) {
        is.read(reinterpret_cast<char *>(material.m_name), 256 * sizeof(char));
        is.read(reinterpret_cast<char *>(&material.m_color), sizeof(Color));
        is.read(reinterpret_cast<char *>(&material.m_roughness), sizeof(float));
        is.read(reinterpret_cast<char *>(&material.m_metallic), sizeof(float));
        is.read(reinterpret_cast<char *>(&material.m_ambientOcclusion), sizeof(float));

        is.read(reinterpret_cast<char *>(&material.m_textureCount), sizeof(grm::u32));

        material.m_textures = new gTexture[material.m_textureCount];
        is.read(reinterpret_cast<char *>(material.m_textures), material.m_textureCount * sizeof(gTexture));

        for (int i=0;i<material.m_textureCount;++i) {
            auto &texture = material.m_textures[i];
        }
    }
        
        // builda a mesh
        auto &meshRender = gameObject->addComponent<MeshRender>();
        for (auto i=0;i<m_materials.size();++i) {
            auto& material = m_materials[i];

            auto &m = meshRender.addMaterial(i, {material.m_name, {material.m_color.r, material.m_color.g, material.m_color.b, material.m_color.a}});

            for (int i=0;i<material.m_textureCount;++i) { 
                auto& texture = material.m_textures[i];

                m.setTexture(i, Resources::LoadTexture(texture.m_path));
            }

            delete[] material.m_textures;
        }
        meshRender.build();
    }

    grm::u32 nSize;
    is.read(reinterpret_cast<char *>(&nSize), sizeof(grm::u32));

    for (int i=0;i<nSize;++i) {
        GameObject *child = ReadChildren(is);
        child->getTransform().setParent(&transform);
    }
    return gameObject;
}

GameObject *Resources::LoadGameObject(const std::string &path) {
    Console::log(LogInfo::INFO, "Reading model...");

    std::ifstream is(path, std::ios_base::binary);
    if (!is.is_open()) {
        Console::log(LogInfo::ERROR, "falied to open file: " + path);
        return nullptr;
    }

    gHeader pHeader;
    if (!is.read(reinterpret_cast<char *>(&pHeader), sizeof(gHeader))) {
        Console::log(LogInfo::ERROR, "Failed to read header to file.");
        return nullptr;
    }

    if (std::string(pHeader.signature) != "grmd") {
        Console::log(LogInfo::ERROR, "Model invalid: " + path);
        return nullptr;
    }

    auto name = Filesystem::GetFileName(path) + "(root)";
    name = FindNewName(name);

    auto gameObject = Scene::Create(name);
    auto &transform = gameObject->getTransform();

    // ler as informaçoes do modelo
    auto child = ReadChildren(is);
    child->getTransform().setParent(&transform);

    // fecha o fluxo de arquivo
    is.close();

    // retorna um gameobject valido
    return gameObject;
}

void Resources::AddShaderLoaded(std::function<bool(grr::gShader *, const std::string &)> func) {
    m_functions.push_back(func);
}

Mesh *Resources::FindMesh(const std::string &name) {
    auto it = s_meshList.find(name);
    if (it == s_meshList.end()) {
        return nullptr;
    }
    return &(*(*it).second);
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
void Resources::SetupShader(grr::gShader *shader, const std::string &frag, const std::string &vert, std::string path) {
    Console::log(LogInfo::WARNING, "Shader loadedd sucessfull.");

    const char *fShader = frag.c_str();
    const char *vShader = vert.c_str();

    grr::gShader::Create(shader, &fShader, &vShader);

    m_functions.erase(std::remove_if(m_functions.begin(), m_functions.end(), [&shader, &path](auto &func) {
        return func(shader, path);
    }), m_functions.end());

    // for (auto &func : m_functions) {
    //     func(shader);
    // }

    s_shaderList.emplace(path, std::unique_ptr<grr::gShader>(shader));
}

void Resources::ProcessShader() {
    std::unique_lock<std::mutex> lock(s_shaderMutex);
    while (!s_shaderFromLoader.empty()) {
        auto [path, shader] = s_shaderFromLoader.front();
        lock.unlock();

        // Processing shader

        std::string frag = path + ".frag";
        std::string vert = path + ".vert";

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

        Event::Add(Resources::SetupShader, shader, fragSource, vertSource, path);

        lock.lock();
        s_shaderFromLoader.pop_front();
    }
}

void Resources::DeleteMesh(Mesh *target) {
    if (!target) {
        return;
    }
    auto it = s_meshList.find(target->getName());
    if (it == s_meshList.end()) {
        return;
    }
    s_meshList.erase(it);
}

void Resources::OnThread() {
    ProcessShader();

    ProcessTexture();
}

const grm::size Resources::GetTextureCount() {
    return s_textureList.size();
}

const grm::size Resources::GetShaderCount() {
    return s_shaderList.size();
}

const grm::size Resources::GetMeshCount() {
    return s_meshList.size();
}
