#include "fbxMesh.hpp"

FbxMesh::FbxMesh(const std::string &name) : m_name(name) {}

const std::vector<FbxVertex> &FbxMesh::getVertices() const {
    return m_vertices;
}

void FbxMesh::addIndice(int idx) {
    m_indices.push_back(idx);
}

const std::vector<int> &FbxMesh::getIndices() const {
    return m_indices;
}

const std::vector<FbxMaterial> &FbxMesh::getMaterials() const {
    return m_materials;
}
