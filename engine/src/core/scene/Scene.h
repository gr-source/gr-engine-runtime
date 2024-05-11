#pragma once

#include "../../common.h"
#include "../../gameObject.h"

class Scene {
public:
    Scene(const std::string& name = "default");

    void add(GameObject* gameobject);

    void remove(const GameObject* gameObject);
private:
    std::vector<GameObject*> m_gameObjects;
    std::string name;

    friend class SceneManager;
};


