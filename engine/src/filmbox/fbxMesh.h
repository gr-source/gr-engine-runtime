#pragma once

#include "fbxVertex3D.h"

class FbxMesh {
public:
    FbxMesh(const std::string& name = "default");

    const std::string& getName() const;

    void addMaterial(u32 index);

    const u32* findMaterial(u32 index);

    const std::vector<u32> getMaterials() const;

    template <typename... Args>
    FbxVertex3D& addVertex(Args&&... args);

    const std::vector<FbxVertex3D>& getVertices() const;

    const ulong getVerticeCount() const;

    void addTriangle(u32 a1, u32 a2, u32 a3);

    const std::vector<u32>& getIndices() const;

    template <typename T>
    void insert(const T& verctor);

protected:
    std::string m_name;

    std::vector<u32> m_material;

    std::vector<u32> m_indices;
    
    std::vector<FbxVertex3D> m_vertices;
};

template <typename... Args>
inline FbxVertex3D &FbxMesh::addVertex(Args &&...args) {
    return m_vertices.emplace_back(std::forward<Args>(args)...);
}
