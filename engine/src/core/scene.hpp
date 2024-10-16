#pragma once

#include "../ecs/gameObject.h"

#include "geometry/material.h"

#include <list>

class Streaming;

class Scene {
public:
    static void SetCurrent(Scene *scene);

    static Scene *GetCurrentScene();

    static bool HasScene();

    static void Use(Scene *scene);
    
    static void Initialize();

    static void Shutdown();

    static Scene *Create(const std::string &name);

    static Scene *LoadScene(const std::string &path);

    Scene(const std::string &name);
    ~Scene();

    const std::string &getName() const;
    
    void save(const std::string &path);

    GameObject &CreateGameObject(const std::string &name, const std::string &tag = "untagged");

    std::vector<GameObject *> &getObjectList();

    void Add(GameObject *gameObject);
    
    static void Remove(GameObject *gameObject);

    static void Delete(GameObject *gameObject);

    void Clear();

    static std::unordered_map<std::string, std::unique_ptr<Scene>> &GetScenes();

    static void DestroyObjects();

    static void ReloadObjects();

private:
    static std::unordered_map<std::string, std::unique_ptr<Scene>> m_scenes;

    static std::hash<std::string> hash;

    static Scene *m_current;

    std::vector<GameObject *> s_objects;

    std::string m_name;

    std::string m_path;

    bool m_enable;

    enum LoadFlags : std::uint8_t {
        LoadFlags_Mesh    = 1 << 1,
        LoadFlags_Object  = 1 << 2,
        LoadFlags_Texture = 1 << 3
    };

    void load(const std::string &path, std::uint8_t flags = LoadFlags_Mesh | LoadFlags_Object | LoadFlags_Texture);

    void setEnable(bool value);

    void setDisableEntity(Transform *transform, bool value);

    int writeScene(Streaming *streaming);

    int writeObject(Streaming *streaming);

    int writeMesh(Streaming *streaming);

    int writeTexture(Streaming *streaming);

    static void WriteGameObject(Streaming *streaming, GameObject *gameObject);

    int readObject(Streaming *streaming);

    int readTexture(Streaming *streaming);

    int readMesh(Streaming *streaming);

    int readNode(Streaming *streaming, Transform *parent = nullptr);

    void remove(GameObject *gameObject);
};
