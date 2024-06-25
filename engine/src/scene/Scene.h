#pragma once

#include "../common.h"
#include "../gameObject.h"
#include "../objectManager.h"

class Scene {
public:
    Scene(const std::string& name = "default");

    template <typename... Args>
    GameObject* add(Args... args);

    void remove(const GameObject* gameObject);
private:
    std::vector<GameObject*> m_gameObjects;
    std::string name;

    friend class SceneManager;
};

template <typename... Args>
inline GameObject* Scene::add(Args... args) {
    return m_gameObjects.emplace_back(&ObjectManager::add(std::forward<Args>(args)...));
}
