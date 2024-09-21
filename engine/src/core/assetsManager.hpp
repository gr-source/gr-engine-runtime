#pragma once

#include <condition_variable>
#include <unordered_set>
#include <functional>
#include <memory>
#include <string>
#include <thread>
#include <vector>
#include <mutex>
#include <list>
#include <map>

#include <gr-render/gRender.h>

#include "core/animation/animation.hpp"
#include "core/binary/gHeader.hpp"
#include "core/file/file.hpp"

#include "geometry/material.h"
#include "geometry/mesh.h"

class FbxAnimation;

class GameObjectHandle {
public:
    GameObjectHandle(std::function<void(GameObject *)> &function, const std::string &path) : m_function(function), m_gameObject(nullptr), m_path(path) {}

    std::function<void(GameObject *)> m_function;

    GameObject *m_gameObject;

    std::string m_path;
};

class AssetsManager {
public:
    static void Initialize();

    static void Shutdown();

    static grr::gTexture *LoadTexture(const std::string &path);

    static grr::gShader *LoadShader(const std::string &name, const std::string &frag, const std::string &vert);

    template <typename ...Args>
    static void LoadGameObject(const std::string &path, Args &&...args);

    static Animation *LoadAnimation(const std::string &path);

    static void AddPostLoader(std::function<bool(void *, const std::string &)> func);

    static grr::gShader *FindShader(const std::string &name);

    static Mesh *FindMesh(const std::string &name);

    template <typename ...Args>
    static Mesh *AddMesh(const std::string &key, Args &&...args);

    static void DeleteMesh(Mesh *target);

    static std::unordered_map<std::string, std::unique_ptr<Mesh>> &GetMeshListMap();

    static std::vector<Material> *FindMaterial(const std::string &name);

    template <typename ...Args>
    static std::vector<Material> &AddMaterial(const std::string &name, Args &&...args);

    static std::unordered_map<std::string, std::vector<Material>> &GetMaterialMap();

    static void OnThread();

    static grm::u32 m_processCount;

    static grm::size GetMaterialCount();

    static grm::size GetTextureCount();

    static grm::size GetShaderCount();
    
    static grm::size GetMeshCount();

    static std::string GetTextureKeyMap(grr::gTexture *texture);

    static std::unordered_map<std::string, std::unique_ptr<grr::gTexture>> &GetTextureMap();

    static void SetGameObjectList(const std::string &path, std::function<void(GameObject *)> &func);

private:
    /***************** Shader *****************/
    static std::unordered_map<std::string, std::unique_ptr<grr::gShader>> s_shaderList;
    static std::list<std::tuple<std::string, std::string, std::string, grr::gShader *>> s_shaderFromLoader;
    static std::mutex s_shaderMutex;

    static std::vector<std::function<bool(void *, const std::string &)>> m_functions;

    /***************** Texture *****************/
    static std::unordered_map<std::string, std::unique_ptr<grr::gTexture>> s_textureList;
    static std::list<std::tuple<std::string, grr::gTexture *>> s_textureFromLoader;
    static std::mutex s_textureMutex;

    /***************** Animation *****************/
    static std::unordered_map<std::string, std::unique_ptr<Animation>> s_animationList;
    static std::list<std::tuple<std::string, Animation *>> s_animationFromLoader;
    static std::mutex s_animatioMutex;

    /***************** GameObject *****************/
    static std::list<GameObjectHandle> s_gameObjectFromLoader;
    // static std::list<std::tuple<std::string, Transform *>> s_gameObjectFromLoader;
    static std::mutex s_gameObjectMutex;

    /***************** Mesh *****************/
    static std::unordered_map<std::string, std::unique_ptr<Mesh>> s_meshList;
    static std::mutex m_meshMutex;
    
    /***************** Material *****************/
    static std::unordered_map<std::string, std::vector<Material>> m_materialMap;

    static std::thread m_thread;

    static void SetupTexture(grr::gTexture *texture, int width, int height, int ch, void *pixels, std::string path);

    static void ProcessTexture();

    static void SetupShader(grr::gShader *shader, const std::string &frag, const std::string &vert, const std::string &path);
    
    static bool CheckShaderError(grr::gShader *shader, const std::string &name);

    static void ProcessShader();

    static void SetupAnimation(const std::string &path, FbxAnimation *temp, Animation *target);

    static void doAnimationInBackground(const std::string &path, Animation *target);

    static void ProcessAnimation();

    static void SetupGameObject(GameObjectHandle &handle, Transform *&out, gModel &model, Mesh *mesh, std::vector<gMaterial> &materials, Transform *parent);

    static void doInBackgroundGameObject(GameObjectHandle &handle);

    static void ProcessBinary();

    static void ReadChildren(Transform *transform, GameObjectHandle &handle);
};

template <typename... Args>
inline void AssetsManager::LoadGameObject(const std::string &path, Args &&...args) {
    std::function<void(GameObject *)> func = std::bind(std::forward<Args>(args)..., std::placeholders::_1);

    SetGameObjectList(path, func);
}

template <typename... Args>
inline Mesh *AssetsManager::AddMesh(const std::string &key, Args &&...args)
{
    std::unique_lock<std::mutex> lock(m_meshMutex);
    auto &&[pair, added] = s_meshList.emplace(key, std::make_unique<Mesh>(std::forward<Args>(args)...));
    auto mesh = &(*(*pair).second);
    lock.unlock();
    return mesh;
}

template <typename... Args>
inline std::vector<Material> &AssetsManager::AddMaterial(const std::string &name, Args &&...args) {
    auto &&[it, added] = m_materialMap.emplace(name, std::forward<Args>(args)...);
    return it->second;
}
