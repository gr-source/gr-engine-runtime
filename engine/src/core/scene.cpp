#include "scene.hpp"

#include "../scriptManager/scriptManager.hpp"

#include "core/file/streaming.hpp"
#include "core/file/file.hpp"

#include "core/binary/gHeader.hpp"

#include "core/assetsManager.hpp"
#include "core/event.hpp"

#include "console.hpp"
#include "settings.h"

#include "script/meshRender.hpp"
#include "script/transform.hpp"
#include "script/camera.hpp"

#include "ecs/entity.h"

std::unordered_map<std::string, std::unique_ptr<Scene>> Scene::m_scenes;

std::hash<std::string> Scene::hash;

std::condition_variable Scene::cv;

std::mutex Scene::m_mutex;

Scene *Scene::m_current = nullptr;

bool Scene::bQuit = false;

void Scene::SetCurrent(Scene *scene) {
    m_current = scene;
}

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

void Scene::Initialize() {
}

void Scene::Shutdown() {
    m_scenes.clear();

    m_current = nullptr;

    {
        std::lock_guard<std::mutex> lock(m_mutex);
        bQuit = true;
    }
    cv.notify_all();
}

Scene *Scene::Create(const std::string &name) {
    auto &&[it, added] = m_scenes.emplace(name, std::make_unique<Scene>(name));

    Scene::Use(&(*it->second));

    try {
        auto &camera = it-> second->CreateGameObject("MainCamera");
        camera.addComponent<Camera>();
    } catch (std::runtime_error e) {
        LOG_ERROR_ARGS("[ SCENE ] Falha ao adiconar uma camera na cena: %s", e.what());
    }
    
    return &(*it->second);
}

#include "core/threadpool.hpp"
void Scene::LoadScene(const std::string &path) {
    Threadpool::AddTask(&Scene::Load, path, LoadFlags_Mesh | LoadFlags_Object | LoadFlags_Texture | LoadFlags_Script, nullptr);
    // std::unique_ptr<Scene> scene = std::make_unique<Scene>(path);

    // if (!scene->load(path)) {
    //     return;
    // }

    // m_scenes.emplace(scene->getName(), std::move(scene));
}

Scene::Scene(const std::string &name) : m_name(name), m_enable(true) {}

Scene::~Scene() {
    for (auto &gameObject : s_objects) {
        GameObject::Destroy(gameObject);
    }
}

const std::string &Scene::getName() const {
    return m_name;
}

void Scene::save(const std::string &path) {
    Streaming stream(path, std::ios_base::out | std::ios_base::binary);

    // header data
    {
        gHeader header{{'g', 'r', 's', 'c'}, 1.0f, static_cast<std::size_t>(stream.getDataSize())};
        std::strncpy(header.name, m_name.data(), 256);

        if (!stream.writeS(&header, 1, false)) {
            return LOG_ERROR("[ Scene ] Failed to write scene: 0");
        }
    }

    if (auto err = writeScene(&stream) != 0) {
        return LOG_ERROR_ARGS("[ SCENE ] Failed to write scene segment: %d", err);
    }

    gStructData end{gHeaderType_None, 0};
    if (!stream.writeS(&end, 1, false)) {
        return LOG_ERROR("[ Scene ] Failed to write scene: 1");
    }

    LOG_INFO_ARGS("[ Scene ] Scene saved: %s", m_name.c_str());
}

GameObject &Scene::CreateGameObject(const std::string &name, const std::string &tag) {
    return *s_objects.emplace_back(&GameObject::Create(name, tag));
}

std::vector<GameObject *> &Scene::getObjectList() {
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
        return LOG_ERROR("[ SCENE ] Failed to delete game object.");
    }
    
    Scene::Remove(gameObject);

    GameObject::Destroy(gameObject);
}

void Scene::Clear() {
    s_objects.clear();
}

std::unordered_map<std::string, std::unique_ptr<Scene>> &Scene::GetScenes() {
    return m_scenes;
}

void Scene::DestroyObjects() {
    auto scene = GetCurrentScene();
    if (!scene) {
        return;
    }

    auto temp_dir = File::GetTempFile("gr-engine/");
    if (!File::Exists(temp_dir)) {
        File::CreateDirectory(temp_dir);
    }

    scene->save(temp_dir + scene->getName() + ".scn.tmp");

    auto &objectList = scene->getObjectList();
    for (auto it = objectList.begin();it != objectList.end();) {
        GameObject::Destroy(*it);

        it = objectList.erase(it);
    }

    GameObject::Clear();
}

void Scene::ReloadObjects() {
    // /*
    auto scene = GetCurrentScene();
    if (!scene) {
        return;
    }

    auto temp_dir = File::GetTempFile("gr-engine/");
    if (!File::Exists(temp_dir)) {
        return;
    }
    Threadpool::AddTask(&Scene::Load, temp_dir + scene->getName() + ".scn.tmp", LoadFlags_Object | LoadFlags_Script, scene);

    // scene->load(temp_dir + scene->getName() + ".scn.tmp", LoadFlags_Object | LoadFlags_Script);
    // */
}

// TODO: Object, Script because thread dead lock
void Scene::Load(const std::string &path, std::uint8_t flags, Scene *scene) {
    if (!File::Exists(path)) {
        LOG_ERROR("[ Scene ] File not found: " + path);
        return;
    }

    Streaming stream(path, std::ios_base::in | std::ios_base::binary);
    if (!stream.is_open()) {
        LOG_ERROR("[ Scene ] Faleid to open scene: " + path);
        return;
    }

    std::string name;

    // read header data
    {
        gHeader header;
        if (!stream.readS(&header, 1)) {
            LOG_ERROR("[ Scene ] Failed to read binary header: " + path);
            return;
        }
        name = header.name;

        LOG_INFO("[ Scene ] loading scene: " + name + " - Version: " + std::to_string(header.version));
    }

    auto hasScene = scene != nullptr;

    scene = (hasScene ? scene : new Scene(name));

    // read segment
    while (true) {
        gStructData data;
        if (!stream.readS(&data, 1)) {
            break;
        }
        
        if (data.type == gHeaderType_None && data.sizeData == 0) {
            break;
        }
        
        auto start_block_position = stream.tellg();
        auto block_name = struct_name[data.type];

        if (data.sizeData <= 0) {
            stream.seekg(start_block_position + static_cast<std::streampos>(data.sizeData), std::ios_base::beg);
            continue;
        }

        int result = -1;
        switch (data.type) {
        case gHeaderType_Mesh:
            if (flags & LoadFlags_Mesh)
                result = readMesh(&stream);
            break;
        case gHeaderType_Texture:
            if (flags & LoadFlags_Texture)
                result = readTexture(&stream);
            break;
        case gHeaderType_Object:
            if (flags & LoadFlags_Object)
                result = readObject(&stream, &(*scene));
            break;
        case gHeaderType_Script:
            if (flags & LoadFlags_Script)
                result = readScript(&stream);
            break;
        default:
            break;
        }

        if (result != 0) {
            LOG_ERROR("[ Scene ] Falied to read segment data: " + std::string(block_name) + " Code: " + std::to_string(result));
        }
        stream.seekg(start_block_position + static_cast<std::streampos>(data.sizeData), std::ios_base::beg);
    }

    Event::Add(&Scene::SetupScene, std::move(scene));
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

int Scene::writeScene(Streaming *streaming) {
    gStructData segment = {gHeaderType_None, 0};

    // textures
    if (writeTexture(streaming) != 0) {
        return 1;
    }

    segment = {gHeaderType_Texture, static_cast<std::size_t>(streaming->getDataSize())};
    if (!streaming->writeS(&segment, 1, false)) {
        return 2;
    }

    streaming->flush();

    // Mesh
    if (writeMesh(streaming) != 0) {
        return 3;
    }

    segment = {gHeaderType_Mesh, static_cast<std::size_t>(streaming->getDataSize())};
    if (!streaming->writeS(&segment, 1, false)) {
        return 3;
    }

    streaming->flush();
    
    // Objects
    if (writeObject(streaming) != 0) {
        return 4;
    }
    
    segment = {gHeaderType_Object, static_cast<std::size_t>(streaming->getDataSize())};
    if (!streaming->writeS(&segment, 1, false)) {
        return 4;
    }

    streaming->flush();

    // script
    if (writeScript(streaming) != 0) {
        return 4;
    }
    
    segment = {gHeaderType_Script, static_cast<std::size_t>(streaming->getDataSize())};
    if (!streaming->writeS(&segment, 1, false)) {
        return 4;
    }

    streaming->flush();
    return 0;
}

int Scene::writeTexture(Streaming *streaming) {
    auto &textureMap = AssetsManager::GetTextureMap();
    auto count = AssetsManager::GetTextureCount();

    if (!streaming->writeS(&count, 1)) {
        return 2;
    }

    for (auto &&[path, ptr] : textureMap) {
        auto size = path.size();

        if (!streaming->writeS(&size, 1)) {
            return 3;
        }

        if (!streaming->writeS(path.data(), size)) {
            return 4;
        }
    }

    return 0;
}

int Scene::writeMesh(Streaming *streaming) {
    auto meshCount = AssetsManager::GetMeshCount();

    streaming->writeS(&meshCount, 1);

    for (auto &&[name, mesh] : AssetsManager::GetMeshListMap()) {
        auto size = name.size();

        streaming->writeS(&size, 1);
        streaming->writeS(name.data(), size);

        mesh->serialize(streaming);
    }
    return 0;
}

int Scene::writeObject(Streaming *streaming) {
    auto objectCount = s_objects.size();

    if (!streaming->writeS(&objectCount, 1)) {
        return 1;
    }

    std::function<void(GameObject *)> func = std::bind(&Scene::writeGameObject, this, streaming, std::placeholders::_1);

    if (func)
        std::for_each(s_objects.begin(), s_objects.end(), func);
    return 0;
}

int Scene::writeScript(Streaming *streaming) {
    auto size = m_componentCache.size();

    streaming->writeS(&size, 1);

    for (auto &&[objectName, componentName, ptr] : m_componentCache) {
        
        // ObjectName
        size = objectName.size();

        streaming->writeS(&size, 1);
        streaming->writeS(objectName.data(), size);

        // ComponentName
        size = componentName.size();

        streaming->writeS(&size, 1);
        streaming->writeS(componentName.data(), size);

        Streaming temp;
        ptr->Serialize(&temp);
        std::size_t data_size = temp.getDataSize();

        streaming->writeS(&data_size, 1);
        ptr->Serialize(streaming);

        temp.clear();
    }
    m_componentCache.clear();
    return 0;
}

int Scene::writeGameObject(Streaming *streaming, GameObject *gameObject) {
    auto name = gameObject->getName();
    auto nSize = name.size();

    auto tag = gameObject->getTag();
    auto tSize = tag.size();

    streaming->writeS(&nSize, 1);
    streaming->writeS(name.data(), nSize);

    streaming->writeS(&tSize, 1);
    streaming->writeS(tag.data(),  tSize);

    auto active = gameObject->getActive();
    auto enable = gameObject->getEnable();
    streaming->writeS(&active,  1);
    streaming->writeS(&enable,  1);

    // Write Component
    auto entity = gameObject->getEntity();

    auto &components = entity->getComponents();
    // auto cSize = components.size();
    // streaming->writeS(&cSize, 1);

    std::list<std::tuple<std::string, Component *>> m_list;
    for (auto &&[name, ptr] : components) {
        m_list.emplace_front(name, &(*ptr));
    }

    for (auto &&[name, ptr] : m_list) {
        m_componentCache.emplace_back(gameObject->getName(), name, ptr);
    }

    auto &transform = gameObject->getTransform();
    auto &children = transform.getChildren();

    auto cSize = children.size();
    streaming->writeS(&cSize, 1);

    for (auto &child : children) {
        auto object = child->getGameObject();

        writeGameObject(streaming, object);
    }
    return 0;
}

int Scene::readObject(Streaming *streaming, Scene *scene) {
    // Objects node
    std::size_t objectCount;
    int result = 0;

    result = streaming->readS(&objectCount, 1) ? 0 : 1;
    if (result != 0) {
        return result;
    }

    for (auto i=0;i<objectCount;i++) {
        result = readNode(streaming, scene);
        if (result != 0) {
            break;
        }
    }

    return result;
}

int Scene::readTexture(Streaming *streaming) {
    std::size_t size = 0;
    int result = 0;

    result = streaming->readS(&size, 1) ? 0 : 1;
    if (result != 0) {
        return result;
    }

    std::vector<std::string> pathList(size);
    for (auto &path : pathList) {
        std::size_t size;

        result = streaming->readS(&size, 1) ? 0 : 2;
        if (result != 0) {
            break;
        }

        path.resize(size, '\0');

        result = streaming->readS(path.data(), size) ? 0 : 3;
        if (result != 0) {
            break;
        }
    }
    
    for (auto &path : pathList) {
        // AssetsManager::LoadTexture(path);
    }
    return 0;
}

// TODO: pode gerarr erro, por favor corrija
int Scene::readMesh(Streaming *streaming) {
    grm::size meshCount = 0;
    if (!streaming->readS(&meshCount, 1)) {
        return 1;
    }

    for (auto i=0;i<meshCount;i++) {
        std::size_t size = 0;
        if (!streaming->readS(&size, 1)) {
            return 2;
        }

        std::string name(size, '\0');
        if (!streaming->readS(name.data(), size)) {
            return 3;
        }

        auto mesh = AssetsManager::AddMesh(name, name);
        mesh->deserialize(streaming);
    }
    return 0;
}

int Scene::readScript(Streaming *streaming) {
    std::size_t nsize = 0;
    std::size_t size = 0;
    int result = 0;

    result = streaming->readS(&size, 1) ? 0 : 1;
    if (result != 0) {
        return result;
    }

    for (int i=0;i<size;i++) {
        // ObjectName
        result = streaming->readS(&nsize, 1) ? 0 : 2;
        if (result != 0) {
            break;
        }

        std::string objectName(nsize, '\0');
        result = streaming->readS(objectName.data(), nsize) ? 0 : 3;
        if (result != 0) {
            break;
        }

        // ComponentName
        result = streaming->readS(&nsize, 1) ? 0 : 4;
        if (result != 0) {
            break;
        }
        
        std::string componentName(nsize, '\0');
        result = streaming->readS(componentName.data(), nsize) ? 0 : 5;
        if (result != 0) {
            break;
        }

        nsize = 0;

        std::size_t data_size = 0;
        result = streaming->readS(&data_size, 1) ? 0 : 6;
        if (result != 0) {
            break;
        }

        Component *component = nullptr;

        Event::Add(&Scene::SetupScript, objectName, componentName, std::ref(cv), std::ref(component));

        std::unique_lock<std::mutex> lock(m_mutex);
        cv.wait(lock, [&component] {
            return component || bQuit;
        });
        if (bQuit) {
            break;
        }
        lock.unlock();
        
        if (data_size <= 0) {
            continue;
        }
        auto position = streaming->tellg() + static_cast<std::streampos>(data_size);

        Event::Add(&Scene::ProcessScript, std::ref(cv), std::ref(component), streaming);

        lock.lock();
        cv.wait(lock, [&component] {
            return !component || bQuit;
        });
        if (bQuit) {
            break;
        }
        lock.unlock();


        // jump to end script buffer
        streaming->seekg(position, std::ios_base::beg);

        /*
        auto gameObject = GameObject::Find(objectName);
        if (!gameObject) {
            continue;
        }

        auto component = gameObject->getEntity()->hasComponent(componentName) ? gameObject->getEntity()->getComponent(componentName) : gameObject->getEntity()->addComponent(componentName);
        if (component)
            component->Deserialize(streaming);
        */
    }
    return result;
}

void CrateGameObject(Transform *&transform, std::condition_variable &cv, std::mutex &mutex, Scene *scene, Transform *parent, bool active, bool enable, std::string name, std::string tag) {
    GameObject *gameObject = nullptr;
    {
        std::lock_guard<std::mutex> lock(mutex);
        gameObject = parent == nullptr ? &scene->CreateGameObject(name, tag) : &GameObject::Create(name, tag);
    }
    
    gameObject->setActive(active);
    gameObject->setEnable(enable);

    {
        std::lock_guard<std::mutex> lock(mutex);
        transform = &gameObject->getTransform();
        transform->setParent(parent);
    }

    cv.notify_one();
}

int Scene::readNode(Streaming *streaming, Scene *scene, Transform *parent) {
    std::size_t nSize;
    int result = 0;
    
    result = streaming->readS(&nSize, 1) ? 0 : 1;
    if (result != 0) {
        return result;
    }

    std::string name(nSize, '\0');
    result = streaming->readS(name.data(), nSize) ? 0 : 2;
    if (result != 0) {
        return result;
    }

    name = String::RemoveNumber(name);

    std::size_t tSize;
    result = streaming->readS(&tSize, 1) ? 0 : 3;
    if (result != 0) {
        return result;
    }

    std::string tag(tSize, '\0');
    result = streaming->readS(tag.data(),  tSize) ? 0 : 4;
    if (result != 0) {
        return result;
    }

    bool active = true;
    bool enable = true;
    result = streaming->readS(&active,  1) ? 0 : 5;
    if (result != 0) {
        return result;
    }

    result = streaming->readS(&enable,  1) ? 0 : 6;
    if (result != 0) {
        return result;
    }

    Transform *transform = nullptr;

    Event::Add(CrateGameObject, std::ref(transform), std::ref(cv), std::ref(m_mutex), std::ref(scene), parent, active, enable, name, tag);

    std::unique_lock<std::mutex> lock(m_mutex);
    cv.wait(lock, [&transform] {
        return transform;
    });
    lock.unlock();

    /*
    auto &gameObject = parent == nullptr ? CreateGameObject(name, tag) : GameObject::Create(name, tag);
    gameObject.setActive(active);
    gameObject.setEnable(enable);

    auto &transform = gameObject.getTransform();
    transform.setParent(parent);
    */

    std::size_t cSize = 0;
    result = streaming->readS(&cSize, 1) ? 0 : 7;
    if (result != 0) {
        return result;
    }

    for (int i=0;i<cSize;i++) {
        result = readNode(streaming, scene, transform);
        if (result != 0) {
            break;
        }
    }
    transform = nullptr;

    return 0;
}

void Scene::remove(GameObject *gameObject) {
    s_objects.erase(std::remove(s_objects.begin(), s_objects.end(), gameObject), s_objects.end());
}

void Scene::SetupScene(Scene *scene) {
    auto it = m_scenes.find(scene->getName());
    if (it == m_scenes.end())
        m_scenes.emplace(scene->getName(), std::unique_ptr<Scene>(scene));

    for (auto &object : scene->getObjectList()) {
        auto entt = object->getEntity();

        entt->init();
    }
}

void Scene::SetupScript(std::string objectName, std::string name, std::condition_variable &cv, Component *&component) {
    auto gameObject = GameObject::Find(objectName);
    if (!gameObject) {
        return;
    }

    auto entt = gameObject->getEntity();
    if (!entt) {
        return;
    }

    try {
        std::lock_guard<std::mutex> lock(m_mutex);
        component = entt->hasComponent(name) ? entt->getComponent(name) : entt->addComponent(name);
    } catch (const std::exception &e) {
        LOG_ERROR_ARGS("[ Scene ] %s", e.what());
    }
    cv.notify_one();
}

void Scene::ProcessScript(std::condition_variable &cv, Component *&component, Streaming *streaming) {
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        try {
            component->Deserialize(streaming);
        } catch (const std::exception &e) {
            LOG_ERROR_ARGS("[ Scene ] %s", e.what());
        }
        component = nullptr;
    }
    cv.notify_one();
}
