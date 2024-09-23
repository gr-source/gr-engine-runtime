#include "core/geometry/mesh.hpp"

Mesh::Mesh(const std::string &name) : m_name(name) {}

std::string &Mesh::getName() {
    return m_name;
}

std::vector<Vertex3D> &Mesh::getVertices() {
    return m_vertices;
}

void Mesh::addTriangles(int t0, int t1, int t2) {
    m_indices.push_back(t0);
    m_indices.push_back(t1);
    m_indices.push_back(t2);
}

void Mesh::addIndice(int t0) {
    m_indices.push_back(t0);
}

const bool Mesh::hasIndices() const {
    return !m_indices.empty();
}

std::vector<int> &Mesh::getIndices() {
    return m_indices;
}

const std::vector<BoneWeight> &Mesh::getBoneWeight() const {
    return m_boneWeight;
}

void Mesh::clear() {
    m_vertices.clear();
    m_indices.clear();
    m_boneWeight.clear();
}
