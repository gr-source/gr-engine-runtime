#pragma once

#include "../ecs/gameObject.h"

#include <list>

class Scene {
public:
    static void CreateScrene(const std::string &name);

    static void LoadScene(const std::string &name);

    static void SetSceneName(const std::string &name);

    static const std::string &GetSceneName();

    static GameObject &CreateGameObject(const std::string &name, const std::string &tag = "untagged");

    static std::vector<GameObject *> &GetGameObjects();

    static void Add(GameObject *gameObject);
    
    static void Remove(GameObject *gameObject);

    static void Clear();

    static void Save();

private:
    static std::vector<GameObject *> s_objects;

    static std::string m_name;

    static void WriteScene();
};
