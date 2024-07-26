#include "mesh.h"

Mesh::Mesh(const std::string &name) : m_name(name) {}

std::vector<Vertex> &Mesh::getVertices() {
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

std::vector<int> &Mesh::getIndices() {
    return m_indices;
}
