#include "scene.hpp"

#include "../scriptManager/scriptManager.hpp"

#include "filesystem/filesystem.h"
#include "filesystem/stream.h"

#include "core/binary/gHeader.hpp"
#include "core/resources.h"

#include "console.hpp"

#include "script/meshRender.hpp"
#include "script/transform.hpp"


std::vector<GameObject *> Scene::s_objects;

std::string Scene::m_name = "default";

void Scene::CreateScrene(const std::string &name) {
    m_name = name;
}

void Scene::LoadScene(const std::string &name) {
    if (!Filesystem::Exists(Filesystem::GetFile("/scenes/" + name))) {
        return Console::log(LogInfo::ERROR, "Falied to load scene, file do not found");
    }

    if (!Stream::open(Filesystem::GetFile("/scenes/" + name), std::ios_base::in | std::ios_base::binary)) {
        return Console::log(LogInfo::ERROR, "Falied to load scene: " + name);
    }

    gHeader header{{'g', 'r', 's', 'c'}, 1.0f};

    if (!Stream::read(&header, 1)) {
        Stream::close();
        return Console::log(LogInfo::ERROR, "Falied from load scene, invalid header.");
    }

    if (header.signature[0] != 'g' && header.signature[1] != 'r' && header.signature[2] != 's' && header.signature[3] != 'c') {
        Stream::close();
        return Console::log(LogInfo::ERROR, "Falied from load scene, invalid binary model.");
    }

    ReadScene();

    Stream::close();
    Console::log(LogInfo::INFO, "Scene loaded suceess full.");
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

void Scene::Save() {
    if (!Stream::open(Filesystem::GetFile("/scenes/" + m_name), std::ios_base::out | std::ios_base::binary)) {
        return Console::log(LogInfo::ERROR, "Falied to save scene: " + m_name);
    }

    gHeader header{{'g', 'r', 's', 'c'}, 1.0f};

    if (!Stream::write(&header, 1)) {
        Stream::close();
        return Console::log(LogInfo::ERROR, "Falied from write header scene.");
    }

    WriteScene();

    Stream::close();

    Console::log(LogInfo::WARNING, "Scene saved: " + m_name);
}

void Scene::WriteScene() {
    // Materials
    auto &materialMap = Resources::GetMaterialMap();
    auto mSize = materialMap.size();

    Stream::write(&mSize, 1);

    for (auto &&[name, material] : materialMap) {
        WriteMaterial(name, material);
    }

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
}

void Scene::WriteMaterial(const std::string &name, std::unordered_map<int, Material> &materials) {
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

void Scene::ReadScene() {
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
}

void Scene::ReadMaterial() {
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

    std::size_t cSize;
    Stream::read(&cSize, 1);

    auto &gameObject = parent == nullptr ? Scene::CreateGameObject(String::RemoveNumber(name), tag) : GameObject::Create(String::RemoveNumber(name), tag);
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
