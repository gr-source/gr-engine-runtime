#pragma once

#include "../filesystem/filesystem.h"

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

#include "geometry/material.h"
#include "geometry/mesh.h"

class FbxAnimation;

class Resources {
public:
    static void Initialize();

    static void Shutdown();

    static grr::gTexture *LoadTexture(const std::string &path);

    static grr::gShader *LoadShader(const std::string &frag, const std::string &vert);

    static GameObject *LoadGameObject(const std::string &path);

    static Animation *LoadAnimation(const std::string &path);

    static void AddPostLoader(std::function<bool(void *, const std::string &)> func);

    static Mesh *FindMesh(const std::string &name);

    template <typename ...Args>
    static Mesh *AddMesh(Args &&...args);

    static void DeleteMesh(Mesh *target);

    static std::unordered_map<std::string, std::unique_ptr<Mesh>> &GetMeshListMap();

    static std::unordered_map<int, Material> *FindMaterial(const std::string &name);

    template <typename ...Args>
    static std::unordered_map<int, Material> &AddMaterial(Args &&...args);

    static std::unordered_map<std::string, std::unordered_map<int, Material>> &GetMaterialMap();

    static void OnThread();

    static grm::u32 m_processCount;

    static const grm::size GetTextureCount();

    static const grm::size GetShaderCount();
    
    static const grm::size GetMeshCount();

private:
    /***************** Shader *****************/
    static std::unordered_map<std::string, std::unique_ptr<grr::gShader>> s_shaderList;
    static std::list<std::tuple<std::tuple<std::string, std::string>, grr::gShader *>> s_shaderFromLoader;
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
    static std::list<std::tuple<std::string, Transform *>> s_gameObjectFromLoader;
    static std::mutex s_gameObjectMutex;

    /***************** Mesh *****************/
    static std::unordered_map<std::string, std::unique_ptr<Mesh>> s_meshList;
    static std::mutex m_meshMutex;
    
    /***************** Material *****************/
    static std::unordered_map<std::string, std::unordered_map<int, Material>> m_materialMap;

    static std::thread m_thread;

    static void SetupTexture(grr::gTexture *texture, int width, int height, int ch, void *pixels, std::string path);

    static void ProcessTexture();

    static void SetupShader(grr::gShader *shader, const std::string &frag, const std::string &vert, const std::string &path);
    
    static bool CheckShaderError(grr::gShader *shader);

    static void ProcessShader();

    static void SetupAnimation(const std::string &path, FbxAnimation *temp, Animation *target);

    static void doAnimationInBackground(const std::string &path, Animation *target);

    static void ProcessAnimation();

    static void SetupGameObject(Transform *&out, gModel &model, Mesh *mesh, std::vector<gMaterial> &materials, Transform *parent);

    static void doInBackgroundGameObject(const std::string &path, Transform *target);

    static void ProcessBinary();

    static void ReadChildren(Transform *transform);

};

template <typename... Args>
inline Mesh *Resources::AddMesh(Args &&...args) {
    std::unique_lock<std::mutex> lock(m_meshMutex);
    auto &&[pair, added] = s_meshList.emplace(std::forward<Args>(args)...);
    auto mesh = &(*(*pair).second);
    lock.unlock();
    return mesh;
}

template <typename... Args>
inline std::unordered_map<int, Material> &Resources::AddMaterial(Args &&...args) {
    auto &&[it, added] = m_materialMap.emplace(std::forward<Args>(args)...);
    return it->second;
}
