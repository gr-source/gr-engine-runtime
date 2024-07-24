#include "fbxMesh.h"

FbxMesh::FbxMesh(const std::string& name) : m_name(name) {}

const std::string &FbxMesh::getName() const {
    return m_name;
}

void FbxMesh::addMaterial(u32 index) {
    m_material.push_back(index);
}

const u32 *FbxMesh::findMaterial(u32 index) {
    auto it = std::find(m_material.begin(), m_material.end(), index);
    if (it != m_material.end()) {
        return &(*it);
    }
    return nullptr;
}

const std::vector<u32> FbxMesh::getMaterials() const {
    return m_material;
}

const std::vector<FbxVertex3D> &FbxMesh::getVertices() const {
    return m_vertices;
}

const ulong FbxMesh::getVerticeCount() const {
    return m_vertices.size();
}

void FbxMesh::addTriangle(u32 a1, u32 a2, u32 a3) {
    m_indices.push_back(a1);
    m_indices.push_back(a2);
    m_indices.push_back(a3);
}

const std::vector<u32> &FbxMesh::getIndices() const {
    return m_indices;
}

template <>
void FbxMesh::insert(const std::vector<u32>& indices) {
    m_indices.insert(m_indices.end(), indices.begin(), indices.end());
}

template <>
void FbxMesh::insert(const std::vector<FbxVertex3D>& vertices) {
    m_vertices.insert(m_vertices.end(), vertices.begin(), vertices.end());
}
