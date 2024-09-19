#include "scene.hpp"

#include "../scriptManager/scriptManager.hpp"

#include "core/file/stream.hpp"
#include "core/file/file.hpp"

#include "core/binary/gHeader.hpp"
#include "core/assetsManager.hpp"

#include "console.hpp"

#include "script/meshRender.hpp"
#include "script/transform.hpp"
#include "script/camera.hpp"

#include "ecs/entity.h"

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

Scene *Scene::Create(const std::string &name) {
    auto scene = new Scene(name);

    Scene::Use(scene);

    try {
        auto &camera = scene->CreateGameObject("MainCamera");
        camera.addComponent<Camera>();
    } catch (std::runtime_error e) {
        std::cout << "Falah ao adiconar uma camera na cena: " << e.what() << std::endl;
    }
    
    return scene;
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
    if (!Stream::open(File::GetAsset("/scene/" + m_name + ".scn"), std::ios_base::out | std::ios_base::binary)) {
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
    GetCurrentScene()->remove(gameObject);
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

    // Mesh
    if (writeMesh() != 0) {
        return 3;
    }

    // Objects
    if (writeObject() != 0) {
        return 4;
    }
    
    return 0;
}

int Scene::writeObject() {
    // Objects node
    auto objectCount = s_objects.size();

    if (!Stream::write(&objectCount, 1)) {
        return 1;
    }

    std::for_each(s_objects.begin(), s_objects.end(), Scene::WriteGameObject);
    return 0;
}

int Scene::writeMaterial() {
    auto count = AssetsManager::GetMaterialCount();
    auto &materials = AssetsManager::GetMaterialMap();

    Stream::write(&count, 1);
    for (auto &&[name, ptr] : materials) {
        auto size = name.size();

        Stream::write(&size, 1);
        Stream::write(name.data(), size);

        size = ptr.size();
        Stream::write(&size, 1);

        for (auto &material : ptr) {
            material.serialize(Stream::GetStream());
        }
    }
    return 0;
}

int Scene::writeTexture() {
    auto &textureMap = AssetsManager::GetTextureMap();
    auto count = AssetsManager::GetTextureCount();

    grm::size offset = 0;
    offset += sizeof(grm::size);
    offset += count * sizeof(std::size_t);
    // offset += count * sizeof(char); // arrumar

    gStructHeader header{gStructType_Texture, offset};
    if (!Stream::write(&header, 1)) {
        return 1;
    }

    if (!Stream::write(&count, 1)) {
        return 2;
    }

    for (auto &&[path, ptr] : textureMap) {
        auto size = path.size();

        if (!Stream::write(&size, 1)) {
            return 3;
        }

        if (!Stream::write(path.data(), size)) {
            return 4;
        }
    }
    return 0;
}

int Scene::writeMesh() {
    auto meshCount = AssetsManager::GetMeshCount();

    Stream::write(&meshCount, 1);

    for (auto &&[name, mesh] : AssetsManager::GetMeshListMap()) {
        auto size = name.size();

        Stream::write(&size, 1);
        Stream::write(name.data(), size);

        mesh->serialize(Stream::GetStream());
    }
    return 0;
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

        ptr->Serialize(Stream::GetStream());
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
    // Texture
    if (readTexture() != 0) {
        return 1;
    }

    // material
    if (readMaterial() != 0) {
        return 2;
    }

    // mesh
    if (readMesh() != 0) {
        return 3;
    }

    // objects
    if (readObject() != 0) {
        return 4;
    }
    return 0;
}

int Scene::readObject() {
    // Objects node
    std::size_t objectCount;
    if (!Stream::read(&objectCount, 1)) {
        return 1;
    }

    for (auto i=0;i<objectCount;i++) {
        ReadNode(nullptr);
    }

    // std::for_each(s_objects.begin(), s_objects.end(), Scene::WriteGameObject);
    return 0;
}

int Scene::readMaterial() {
    grm::size count = 0;
    Stream::read(&count, 1);

    for (auto i=0;i<count;i++) {
        std::size_t size;
        Stream::read(&size, 1);

        std::string name(size, '\0');
        Stream::read(name.data(), size);

        Stream::read(&size, 1);

        std::vector<Material> materials(size);
        for (auto &material : materials) {
            material.deserialize(Stream::GetStream());
        }

        if (!name.empty() || !materials.empty() && !AssetsManager::FindMaterial(name)) {
            AssetsManager::AddMaterial(name, materials);
        }
    }
    return 0;
}

int Scene::readTexture() {
    gStructHeader header{gStructType_None, 0};
    if (!Stream::read(&header, 1)) {
        return 1;
    }

    std::size_t size = 0;
    if (!Stream::read(&size, 1)) {
        return 2;
    }

    std::vector<std::string> pathList(size);
    for (auto &path : pathList) {
        std::size_t size;
        if (!Stream::read(&size, 1)) {
            return 3;
        }

        path.resize(size, '\0');
        if (!Stream::read(path.data(), size)) {
            return 4;
        }
    }
    

    for (auto &path : pathList) {
        AssetsManager::LoadTexture(path);
    }
    return 0;
}

int Scene::readMesh() {
    grm::size meshCount = 0;
    Stream::read(&meshCount, 1);

    for (auto i=0;i<meshCount;i++) {
        std::size_t size = 0;
        Stream::read(&size, 1);

        std::string name(size, '\0');
        Stream::read(name.data(), size);

        auto mesh = AssetsManager::AddMesh(name, name);
        mesh->deserialize(Stream::GetStream());
    }
    return 0;
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

            component->Deserialize(Stream::GetStream());

            continue;
        }

        auto component = entt->addComponent(name);
        component->Deserialize(Stream::GetStream());
    }

    Stream::read(&cSize, 1);

    for (int i=0;i<cSize;i++) {
        ReadNode(&transform);
    }
}

void Scene::remove(GameObject *gameObject) {
    s_objects.erase(std::remove(s_objects.begin(), s_objects.end(), gameObject), s_objects.end());
}
