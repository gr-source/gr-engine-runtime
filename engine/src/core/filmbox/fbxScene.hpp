#pragma once

#include "fbxMaterial.hpp"
#include "fbxMesh.hpp"
#include "fbxNode.hpp"

#include <vector>

class FbxScene {
public:
    FbxScene(const std::string &name = "default");

    template <typename ...Args>
    FbxMaterial &addMaterial(Args &&...args);

    FbxMaterial &getMaterial(int idx);

    const std::size_t getMaterialCount() const;

    template <typename ...Args>
    FbxMesh &addMesh(Args &&...args);

    FbxMesh &getMesh(int idx);

    const std::size_t getMeshCount() const;

    const FbxNode& getRootNode() const;

    const std::string &getName() const;
    
private:
    std::string m_name;

    std::vector<FbxMaterial> m_materials;

    std::vector<FbxMesh> m_meshes;

    FbxNode m_rootNode;

    friend class Filmbox;
};

template <typename... Args>
inline FbxMaterial &FbxScene::addMaterial(Args &&...args) {
    return m_materials.emplace_back(std::forward<Args>(args)...);
}

template <typename... Args>
inline FbxMesh &FbxScene::addMesh(Args &&...args) {
    return m_meshes.emplace_back(std::forward<Args>(args)...);
}


