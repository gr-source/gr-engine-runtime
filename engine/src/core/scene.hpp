#pragma once

#include "../ecs/gameObject.h"

#include "geometry/material.h"

#include <list>

class Scene {
public:
    static Scene *GetCurrentScene();

    static bool HasScene();

    static void Use(Scene *scene);
    
    Scene(const std::string &name);
    ~Scene();

    const std::string &getName() const;
    
    void load(const std::string &path);

    void save();

    void SetSceneName(const std::string &name);

    const std::string &GetSceneName();

    GameObject &CreateGameObject(const std::string &name, const std::string &tag = "untagged");

    std::vector<GameObject *> &GetGameObjects();

    void Add(GameObject *gameObject);
    
    void Remove(GameObject *gameObject);

    void Delete(GameObject *gameObject);

    void Clear();

private:    
    static Scene *m_current;

    std::vector<GameObject *> s_objects;

    std::string m_name;

    bool m_enable;

    void setEnable(bool value);

    void setDisableEntity(Transform *transform, bool value);

    int writeScene();

    int writeObject();

    int writeMesh();

    int writeMaterial();

    int writeTexture();

    static void WriteGameObject(GameObject *gameObject);

    int readScene();

    int readObject();

    int readMaterial();

    int readTexture();

    int readMesh();

    static void ReadNode(Transform *parent = nullptr);
};
