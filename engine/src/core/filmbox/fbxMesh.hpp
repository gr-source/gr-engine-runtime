#pragma once

#include "fbxMaterial.hpp"
#include "fbxVertex.cpp"

#include <vector>
#include <string>

class FbxMesh {
public:
    FbxMesh(const std::string &name = "default");
    
    template <typename ...Args>
    FbxVertex &addVertice(Args &&...args);

    const std::vector<FbxVertex> &getVertices() const;

    void addIndice(int idx);

    const std::vector<int> &getIndices() const;

    template <typename ...Args>
    FbxMaterial &addMaterial(Args &&...args);

    const std::vector<FbxMaterial> &getMaterials() const;
    
private:
    std::string m_name;

    std::vector<FbxVertex> m_vertices;

    std::vector<int> m_indices;

    std::vector<FbxMaterial> m_materials;
};

template <typename... Args>
inline FbxVertex &FbxMesh::addVertice(Args &&...args) {
    return m_vertices.emplace_back(std::forward<Args>(args)...);
}

template <typename... Args>
inline FbxMaterial &FbxMesh::addMaterial(Args &&...args) {
    return m_materials.emplace_back(std::forward<Args>(args)...);
}
