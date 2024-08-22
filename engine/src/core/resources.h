#pragma once

#include "../filesystem/filesystem.h"

#include <condition_variable>
#include <unordered_set>
#include <functional>
#include <memory>
#include <string>
#include <vector>
#include <mutex>
#include <list>
#include <map>

#include <gr-render/gRender.h>

#include "core/animation/animation.hpp"
#include "core/binary/gHeader.hpp"

#include "geometry/mesh.h"

class FbxAnimation;

class Resources {
public:
    static void Initialize();
    
    static grr::gTexture *LoadTexture(const std::string &path);

    static grr::gShader *LoadShader(const std::string &path);

    static GameObject *LoadGameObject(const std::string &path);

    static Animation *LoadAnimation(const std::string &path);

    static void AddShaderLoaded(std::function<bool(grr::gShader *, const std::string &)> func);

    static Mesh *FindMesh(const std::string &name);

    template <typename ...Args>
    static Mesh *AddMesh(Args &&...args);

    static void DeleteMesh(Mesh *target);

    static void OnThread();

    static grm::u32 m_processCount;

    static const grm::size GetTextureCount();

    static const grm::size GetShaderCount();
    
    static const grm::size GetMeshCount();

private:
    /***************** Shader *****************/
    static std::unordered_map<std::string, std::unique_ptr<grr::gShader>> s_shaderList;
    static std::list<std::tuple<std::string, grr::gShader *>> s_shaderFromLoader;
    static std::mutex s_shaderMutex;

    static std::vector<std::function<bool(grr::gShader *, const std::string &)>> m_functions;

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

    static void SetupTexture(grr::gTexture *texture, int width, int height, int ch, void *pixels, std::string path);

    static void ProcessTexture();

    static void SetupShader(grr::gShader *shader, const std::string &frag, const std::string &vert, std::string path);

    static void ProcessShader();

    static void SetupAnimation(const std::string &path, FbxAnimation *temp, Animation *target);

    static void doAnimationInBackground(const std::string &path, Animation *target);

    static void ProcessAnimation();

    static void SetupGameObject(GameObject *gameObject, gModel &model, Mesh *mesh, std::vector<gMaterial> &materials, Transform *parent);

    static void doInBackgroundGameObject(const std::string &path, Transform *target);

    static void ProcessBinary();

    static void ReadChildren(Transform *transform);

    static std::unordered_map<std::string, grm::size> m_nameCount;
};

template <typename... Args>
inline Mesh *Resources::AddMesh(Args &&...args) {
    auto &&[pair, added] = s_meshList.emplace(std::forward<Args>(args)...);
    return &(*(*pair).second);
}
