#pragma once

#include <string>

#include "fbxCamera.h"
#include "fbxLight.h"
#include "fbxNode.h"

#include "../animation/animationClip.h"
#include "../animation/skeleton.h"

class FbxScene {
public:
    FbxScene(const std::string& name = "MainScene");

    const FbxNode& getroot() const;

    void setname(const std::string& value);

    const std::string& getname() const;

    const FbxLight* findlight(const std::string& name) const;

    const FbxCamera* findcamera(const std::string& name) const;

    template <typename... Args>
    Material& addmaterial(u32 index, Args&& ...args);

    const std::unordered_map<u32, Material>& getmaterials() const;

    template <typename... Args>
    Mesh& addmesh(u32 index, Args&&... args);

    Mesh* findmesh(u32 index);

    const std::vector<AnimationClip>& getanimations() const;

    template <typename... Args>
    AnimationClip& addanimation(Args&&... args);

    template <typename... Args>
    Skeleton& addskeleton(Args&&... args);

    Skeleton* findSkeleton(const std::string& name);

private:
    /* nome da cena */
    std::string name;

    /* raiz principal do nó */
    FbxNode root;

    /* lista de meshs na cena */
    std::unordered_map<u32, Mesh> m_meshes;

    /* lista de materials na cena */
    std::unordered_map<u32, Material> m_materials;

    /* lista de animações na cena */
    std::vector<AnimationClip> m_animations;

    std::unordered_map<std::string, FbxLight> m_lights;

    std::unordered_map<std::string, FbxCamera> m_cameras;

    std::vector<Skeleton> m_skeletons;

    friend class Filmbox;
};

template <typename... Args>
inline Material &FbxScene::addmaterial(u32 index, Args &&...args) {
    m_materials.emplace(index, std::forward<Args>(args)...);
    return m_materials[index];
}

template <typename... Args>
inline Mesh &FbxScene::addmesh(u32 index, Args &&...args) {
    m_meshes.emplace(index, std::forward<Args>(args)...);
    return m_meshes[index];
}

template <typename... Args>
inline AnimationClip &FbxScene::addanimation(Args &&...args) {
    return m_animations.emplace_back(std::forward<Args>(args)...);
}

template <typename... Args>
inline Skeleton &FbxScene::addskeleton(Args &&...args) {
    return m_skeletons.emplace_back(std::forward<Args>(args)...);
}
