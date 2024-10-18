#pragma once

#include "../ecs/gameObject.h"

#include "geometry/material.h"

#include <condition_variable>
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

    static void LoadScene(const std::string &path);

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

    static std::mutex m_mutex;

    static Scene *m_current;

    std::vector<std::tuple<std::string, std::string, Component *>> m_componentCache; 

    std::vector<GameObject *> s_objects;

    std::string m_name;

    std::string m_path;

    bool m_enable;

    enum LoadFlags : std::uint8_t {
        LoadFlags_Mesh    = 1 << 1,
        LoadFlags_Object  = 1 << 2,
        LoadFlags_Texture = 1 << 3,
        LoadFlags_Script  = 1 << 4,
    };

    static void Load(const std::string &path, std::uint8_t flags = LoadFlags_Mesh | LoadFlags_Object | LoadFlags_Texture | LoadFlags_Script, Scene *scene = nullptr);

    void setEnable(bool value);

    void setDisableEntity(Transform *transform, bool value);

    int writeScene(Streaming *streaming);

    int writeTexture(Streaming *streaming);

    int writeMesh(Streaming *streaming);

    int writeObject(Streaming *streaming);

    int writeScript(Streaming *streaming);

    int writeGameObject(Streaming *streaming, GameObject *gameObject);

    static int readObject(Streaming *streaming, Scene *scene);

    static int readTexture(Streaming *streaming);

    static int readMesh(Streaming *streaming);

    static int readScript(Streaming *streaming);

    static int readNode(Streaming *streaming, Scene *scene, Transform *parent = nullptr);

    void remove(GameObject *gameObject);

    static void SetupScene(Scene *scene);

    static void SetupScript(std::string objectName, std::string name, std::condition_variable &cv, Component *&component);

    static void ProcessScript(std::condition_variable &cv, Component *&component, Streaming *streaming);
};
