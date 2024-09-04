#include "fbxMesh.hpp"

FbxMesh::FbxMesh(const std::string &name) : m_name(name) {}

const std::string &FbxMesh::getName() const {
    return m_name;
}

FbxVertex &FbxMesh::getVertice(int idx) {
    return m_vertices[idx];
}

const std::vector<FbxVertex> &FbxMesh::getVertices() const {
    return m_vertices;
}

void FbxMesh::addIndice(int idx) {
    m_indices.push_back(idx);
}

void FbxMesh::addTriangle(int ia, int ib, int ic) {
    m_indices.push_back(ia);
    m_indices.push_back(ib);
    m_indices.push_back(ic);
}

const std::vector<int> &FbxMesh::getIndices() const {
    return m_indices;
}

grm::size FbxMesh::addMaterial(int idx) {
    m_materials.push_back(idx);
    return m_materials.size() - 1;
}

const std::vector<int> &FbxMesh::getMaterials() const {
    return m_materials;
}

const std::vector<FbxBone> &FbxMesh::getBones() const {
    return m_bones;
}

const bool FbxMesh::hasBone() const {
    return !m_bones.empty();
}
