#include "scene.hpp"

#include "../scriptManager/scriptManager.hpp"

#include "filesystem/filesystem.h"
#include "filesystem/stream.h"

#include "core/binary/gHeader.hpp"

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
    std::cout << "Deleting: " << gameObject->getName() << std::endl;
    
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
}
