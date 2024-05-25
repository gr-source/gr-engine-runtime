#pragma once

#include "Scene.h"

class SceneManager {
public:
    static void Save(const std::string& path);

    static void Load(const std::string& path);
    
    template <typename... Args>
    static GameObject* add(Args&&... args);

    static void remove(GameObject* gameobject);

    static const std::string& GetName();

    static std::vector<GameObject*>& GetObjects();
private:
    static Scene m_scene;
};

template <typename... Args>
inline GameObject* SceneManager::add(Args &&...args) {
    return m_scene.add(std::forward<Args>(args)...);
}
