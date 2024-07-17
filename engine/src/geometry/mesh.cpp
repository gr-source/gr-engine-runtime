#include "mesh.h"

#include "../filesystem/stream.h"

Mesh::Mesh(const std::string& name) : m_name(name) {}

const std::string& Mesh::getname() const {
    return m_name;
}

const std::vector<Vertex3D>& Mesh::getvertices() const {
    return m_vertices;
}

Vertex3D& Mesh::GetVertex(u32 index) {
    return m_vertices[index];
}

const u32 Mesh::getVerticeCount() const {
    return m_vertices.size();
}

const Vertex3D *Mesh::getVerticeData() const {
    return m_vertices.data();
}

void Mesh::addindice(u32 indice) {
    m_indices.emplace_back(indice);
}

void Mesh::addindex(u32 indice) {
    m_indices.emplace_back(indice);
}

void Mesh::addTriangle(u32 t0, u32 t1, u32 t2) {
    m_indices.emplace_back(t0);
    m_indices.emplace_back(t1);
    m_indices.emplace_back(t2);
}

const std::vector<u32>& Mesh::getindices() const {
    return m_indices;
}

const u32 Mesh::getIndiceCount() const {
    return m_indices.size();
}

const u32 *Mesh::getIndiceData() const {
    return m_indices.data();
}

u32& Mesh::GetIndice(u32 index) {
    return m_indices[index];
}

const bool Mesh::hasindices() const {
    return !m_indices.empty();
}

bool Mesh::operator ==(const Mesh& other) const {
    return this == &other;
}

void Mesh::addMaterial(u32 index) {
    m_material.emplace_back(index);
}

const std::vector<u32> &Mesh::getMaterials() const {
    return m_material;
}

u32 *Mesh::findMaterial(u32 index) {
    auto it = std::find(m_material.begin(), m_material.end(), index);
    if (it != m_material.end()) {
        return &(*it);
    }
    return nullptr;
}

template <>
void Mesh::insert(const std::vector<Vertex3D>& vector) {
    m_vertices.insert(m_vertices.begin(), vector.begin(), vector.end());
}

template <>
void Mesh::insert(const std::vector<u32>& vector) {
    m_indices.insert(m_indices.begin(), vector.begin(), vector.end());
}

void Mesh::clear() {
    m_indices.clear();
    m_vertices.clear();
}
