#pragma once

#include "fbxAnimation.hpp"
#include "fbxMaterial.hpp"
#include "fbxMesh.hpp"
#include "fbxNode.hpp"

#include <algorithm>

class FbxScene {
public:
    FbxScene(const std::string &name = "default");

    template <typename ...Args>
    FbxAnimation &addAnimation(Args &&...args);

    const std::vector<FbxAnimation> &getAnimations() const;

    template <typename ...Args>
    FbxMaterial &addMaterial(Args &&...args);

    const FbxMaterial &getMaterial(int idx) const;

    const std::size_t getMaterialCount() const;

    template <typename ...Args>
    FbxMesh &addMesh(Args &&...args);

    const FbxMesh &getMesh(int idx) const;

    FbxMesh *findMesh(const std::string &name);

    const std::size_t getMeshCount() const;

    const FbxNode& getRootNode() const;

    const std::string &getName() const;
    
private:
    std::string m_name;

    std::vector<FbxAnimation> m_animation;

    std::vector<FbxMaterial> m_materials;

    std::vector<FbxMesh> m_meshes;

    FbxNode m_rootNode;

    friend class Filmbox;
};

template <typename... Args>
inline FbxAnimation &FbxScene::addAnimation(Args &&...args) {
    return m_animation.emplace_back(std::forward<Args>(args)...);
}

template <typename... Args>
inline FbxMaterial &FbxScene::addMaterial(Args &&...args) {
    return m_materials.emplace_back(std::forward<Args>(args)...);
}

template <typename... Args>
inline FbxMesh &FbxScene::addMesh(Args &&...args) {
    return m_meshes.emplace_back(std::forward<Args>(args)...);
}


