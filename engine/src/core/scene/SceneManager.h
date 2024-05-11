#pragma once

#include "Scene.h"

class SceneManager {
public:
    static void Save(const std::string& path);

    static void Load(const std::string& path);
    
    static void Add(GameObject* gameobject);

    static const std::string& GetName();

    static std::vector<GameObject*>& GetObjects();
private:
    static Scene m_scene;
};

