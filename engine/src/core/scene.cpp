#include "scene.hpp"

#include "../scriptManager/scriptManager.hpp"

#include "core/file/stream.hpp"
#include "core/file/file.hpp"

#include "core/binary/gHeader.hpp"
#include "core/resources.h"

#include "console.hpp"

#include "script/meshRender.hpp"
#include "script/transform.hpp"
#include "script/camera.hpp"

Scene *Scene::m_current = nullptr;

Scene *Scene::GetCurrentScene() {
    return m_current;
}

bool Scene::HasScene() {
    return m_current != nullptr;
}

void Scene::Use(Scene *scene) {
    if (m_current && m_current != scene && m_current->m_enable) {
        m_current->setEnable(false);
    }
    if (scene && scene != m_current && !scene->m_enable) {
        scene->setEnable(true);
    }
    m_current = scene;
}

Scene::Scene(const std::string &name) : m_name(name), m_enable(true) {
}

Scene::~Scene() {
    for (auto &gameObject : s_objects) {
        GameObject::Destroy(gameObject);
    }
}

const std::string &Scene::getName() const {
    return m_name;
}

void Scene::load(const std::string &path) {
    Scene::Use(this);
    
    if (!File::Exists(path)) {
        return Console::log(LogInfo::ERROR, "Falied to load scene, file do not found.");
    }

    if (!Stream::open(path, std::ios_base::in | std::ios_base::binary)) {
        return Console::log(LogInfo::ERROR, "Falied to load scene: " + path);
    }

    gHeader header{{'g', 'r', 's', 'c'}, 1.0f};

    if (!Stream::read(&header, 1)) {
        Stream::close();
        Console::log(LogInfo::ERROR, "Failed to read scene data RS(0)");
    }

    if (header.signature[0] != 'g' && header.signature[1] != 'r' && header.signature[2] != 's' && header.signature[3] != 'c') {
        Stream::close();
        return Console::log(LogInfo::ERROR, "Falied ffrom loader, binary do not is of scene.");
    }

    std::size_t nSize;
    if (!Stream::read(&nSize, 1)) {
        Stream::close();
        return Console::log(LogInfo::ERROR, "Failed to read scene data RS(1)");
    }

    m_name.resize(nSize, '\0');
    if (!Stream::read(m_name.data(), nSize)) {
        Stream::close();
        return Console::log(LogInfo::ERROR, "Failed to read scene data RS(2)");
    }

    if (auto err = readScene() != 0) {
        Stream::close();
        return Console::log(LogInfo::ERROR, "Failed to read scene data child(" + std::to_string(err) + ")");
    }

    Stream::close();
    Console::log(LogInfo::INFO, "Scene loaded suceess full.");
}

void Scene::save() {
    if (!Stream::open(File::GetFile("/scenes/" + m_name + ".scn"), std::ios_base::out | std::ios_base::binary)) {
        return Console::log(LogInfo::ERROR, "Falied to save scene: " + m_name);
    }

    gHeader header{{'g', 'r', 's', 'c'}, 1.0f};
    if (!Stream::write(&header, 1)) {
        Stream::close();
        return Console::log(LogInfo::ERROR, "Failed to write scene data WS(0)");
    }

    auto nSize = m_name.size();
    if (!Stream::write(&nSize, 1)) {
        Stream::close();
        return Console::log(LogInfo::ERROR, "Failed to write scene data WS(1)");
    }

    if (!Stream::write(m_name.data(), nSize)) {
        Stream::close();
        return Console::log(LogInfo::ERROR, "Failed to write scene data WS(2)");
    }

    if (auto err = writeScene() != 0) {
        Stream::close();
        return Console::log(LogInfo::ERROR, "Failed to write scene data child(" + std::to_string(err));
    }

    Stream::close();

    Console::log(LogInfo::INFO, "Scene saved: " + m_name);
}

void Scene::SetSceneName(const std::string &name) {
    m_name = name;
}

const std::string &Scene::GetSceneName() {
    return m_name;
}

GameObject &Scene::CreateGameObject(const std::string &name, const std::string &tag) {
    return *s_objects.emplace_back(&GameObject::Create(name, tag));
}

std::vector<GameObject *> &Scene::GetGameObjects() {
    return s_objects;
}

void Scene::Add(GameObject *gameObject) {
    auto it = std::find(s_objects.begin(), s_objects.end(), gameObject);
    if (it != s_objects.end()) {
        return;
    }
    s_objects.push_back(gameObject);
}

void Scene::Remove(GameObject *gameObject) {
    s_objects.erase(std::remove(s_objects.begin(), s_objects.end(), gameObject), s_objects.end());
}

void Scene::Delete(GameObject *gameObject) {
    if (!gameObject) {
        return Console::log(LogInfo::ERROR, "Failed to delete game object.");
    }
    
    Scene::Remove(gameObject);

    GameObject::Destroy(gameObject);
}

void Scene::Clear() {
    s_objects.clear();
}

void Scene::setEnable(bool value) {
    for (auto &gameObject : s_objects) {
        setDisableEntity(&gameObject->getTransform(), value);
    }
    m_enable = value;
}

void Scene::setDisableEntity(Transform *transform, bool value) {
    transform->entity->setEnable(value);

    for (auto &child : transform->getChildren()) {
        setDisableEntity(child, value);
    }
}

int Scene::writeScene() {
    // textures
    if (writeTexture() != 0) {
        return 1;
    }

    // Material
    if (writeMaterial() != 0) {
        return 2;
    }

    return 0;
    
    /*
    // Meshes
    auto meshCount = Resources::GetMeshCount();

    Stream::write(&meshCount, 1);

    auto &meshList = Resources::GetMeshListMap();
    for (auto &&[name, mesh] : meshList) {
        WriteMesh(name, &(*mesh));
    }

    // Objects node
    auto objectCount = s_objects.size();

    if (!Stream::write(&objectCount, 1)) {
        //
    }

    std::for_each(s_objects.begin(), s_objects.end(), Scene::WriteGameObject);
    */
}

int Scene::writeMaterial() {
    auto count = Resources::GetTextureCount();
    auto &materials = Resources::GetMaterialMap();
    grm::size offset = 0;

    //Write data
    gStructHeader header{gStructType_Material, offset};
    // Materials
    /*
    auto &materialMap = Resources::GetMaterialMap();
    auto mSize = materialMap.size();

    Stream::write(&mSize, 1);

    for (auto &&[name, material] : materialMap) {
        auto sName = name.size();
        Stream::write(&sName, 1);
        Stream::write(name.data(), sName);

        auto mSize = materials.size();
        Stream::write(&mSize, 1);

        for (auto &&[idx, material] : materials) {
            Stream::write(&idx, 1);

            material.serialize(Stream::stream);
        }
    }
    */
    //
    /*
    auto sName = name.size();
    Stream::write(&sName, 1);
    Stream::write(name.data(), sName);

    auto mSize = materials.size();
    Stream::write(&mSize, 1);

    for (auto &&[idx, material] : materials) {
        Stream::write(&idx, 1);

        material.serialize(Stream::stream);
    }
    */
}

int Scene::writeTexture() {
    auto &textureMap = Resources::GetTextureMap();

    std::vector<const char *> pathList;
    for (auto &&[path, ptr] : textureMap) {
        pathList.push_back(path.c_str());
    }

    grm::size offset = 0;

    auto size = pathList.size();
    offset += sizeof(size);
    offset += size * sizeof(const char *);

    gStructHeader header{gStructType_Texture, offset};
    if (!Stream::write(&header, 1)) {
        return 1;
    }

    if (!Stream::write(&size, 1)) {
        return 2;
    }

    if (!Stream::write(pathList.data(), pathList.size())) {
        return 3;
    }
    return 0;
}

void Scene::WriteMesh(const std::string &name, Mesh *mesh) {
    auto nSize = name.size();

    Stream::write(&nSize, 1);
    Stream::write(name.data(), nSize);

    Mesh::Serialize(Stream::stream, mesh);
}

void Scene::WriteGameObject(GameObject *gameObject) {
    auto name = gameObject->getName();
    auto nSize = name.size();

    auto tag = gameObject->getTag();
    auto tSize = tag.size();

    Stream::write(&nSize, 1);
    Stream::write(name.data(), nSize);

    Stream::write(&tSize, 1);
    Stream::write(tag.data(),  tSize);

    auto active = gameObject->getActive();
    auto enable = gameObject->getEnable();
    Stream::write(&active,  1);
    Stream::write(&enable,  1);

    // Write Component
    auto entity = gameObject->getEntity();

    auto &components = entity->getComponents();
    auto cSize = components.size();
    Stream::write(&cSize, 1);

    std::list<std::tuple<std::string, Component *>> m_list;
    for (auto &&[name, ptr] : components) {
        m_list.emplace_front(name, &(*ptr));
    }

    for (auto &&[name, ptr] : m_list) {
        nSize = name.size();
        Stream::write(&nSize, 1);
        Stream::write(name.data(), nSize);

        ptr->Serialize(Stream::stream);
    }

    auto &transform = gameObject->getTransform();
    auto &children = transform.getChildren();

    cSize = children.size();
    Stream::write(&cSize, 1);

    for (auto &child : children) {
        auto object = child->getGameObject();

        WriteGameObject(object);
    }
}

int Scene::readScene() {
    // Textures
    if (readTextures() != 0) {
        return 1;
    }

    return 0;

    /*std::size_t tSize;
    Stream::read(&tSize, 1);

    for (int i=0;i<tSize;i++) {
        std::size_t pSize;
        Stream::read(&pSize, 1);

        std::string path(pSize, '\0');
        Stream::read(path.data(), pSize);

        Resources::LoadTexture(path);
    }

    // Materials
    std::size_t mSize;
    Stream::read(&mSize, 1);

    for (int i=0;i<mSize;i++) {
        ReadMaterial();
    }

    // Meshes
    grm::size meshCount;
    Stream::read(&meshCount, 1);

    for (int i=0;i<meshCount;i++) {
        ReadMesh();
    }

    // gameObject
    std::size_t objectCount;

    if (!Stream::read(&objectCount, 1)) {
        //
    }

    for (std::size_t i=0;i<objectCount;i++) {
        ReadNode();
    }
    */
}

int Scene::readTextures() {
    gStructHeader header{gStructType_None, 0};
    if (!Stream::read(&header, 1)) {
        return 1;
    }

    std::size_t size = 0;
    if (!Stream::read(&size, 1)) {
        return 2;
    }

    std::vector<const char *> pathList(size, nullptr);
    if (!Stream::read(pathList.data(), pathList.size())) {
        return 3;
    }

    for (auto &path : pathList) {
    }
    return 0;
}

void Scene::ReadMaterial() {
    /*
    std::size_t sName;
    Stream::read(&sName, 1);

    std::string name(sName, '\0');
    Stream::read(name.data(), sName);

    std::size_t mSize;
    Stream::read(&mSize, 1);

    std::unordered_map<int, Material> materials;
    for (int i=0;i<mSize;i++) {
        int idx;
        Stream::read(&idx, 1);

        auto &&[it, added] = materials.emplace(idx, "default");

        it->second.deserialize(Stream::stream);
    }
    Resources::AddMaterial(name, materials);
    */
}

void Scene::ReadMesh() {
    std::size_t nSize;

    Stream::read(&nSize, 1);

    std::string name(nSize, '\0');
    Stream::read(name.data(), nSize);

    auto ptr = Resources::AddMesh(name, std::make_unique<Mesh>(name));
    Mesh::Deserialize(Stream::stream, ptr);
}

void Scene::ReadNode(Transform *parent) {
    std::size_t nSize;
    Stream::read(&nSize, 1);

    std::string name(nSize, '\0');
    Stream::read(name.data(), nSize);

    std::size_t tSize;
    Stream::read(&tSize, 1);

    std::string tag(tSize, '\0');
    Stream::read(tag.data(),  tSize);

    bool active = true;
    bool enable = true;
    Stream::read(&active,  1);
    Stream::read(&enable,  1);

    std::size_t cSize;
    Stream::read(&cSize, 1);

    auto &gameObject = parent == nullptr ? Scene::GetCurrentScene()->CreateGameObject(String::RemoveNumber(name), tag) : GameObject::Create(String::RemoveNumber(name), tag);
    gameObject.setActive(active);
    gameObject.setEnable(enable);

    auto &transform = gameObject.getTransform();
    transform.setParent(parent);

    auto entt = gameObject.getEntity();

    for (std::size_t j=0;j<cSize;j++) {
        Stream::read(&nSize, 1);

        name = std::string(nSize, '\0');
        Stream::read(name.data(), nSize);

        if (entt->hasComponent(name)) {
            auto component = entt->getComponent(name);

            component->Deserialize(Stream::stream);

            continue;
        }

        auto component = entt->addComponent(name);
        component->Deserialize(Stream::stream);
    }

    Stream::read(&cSize, 1);

    for (int i=0;i<cSize;i++) {
        ReadNode(&transform);
    }
}
