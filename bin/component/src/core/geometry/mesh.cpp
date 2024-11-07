#include "core/geometry/mesh.hpp"

#include "core/function.hpp"

std::string Mesh::getName() const {
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

bool Mesh::hasIndices() const {
    return !m_indices.empty();
}

std::vector<int> &Mesh::getIndices() {
    return m_indices;
}

std::vector<BoneWeight> Mesh::getBoneWeight() const {
    return m_boneWeight;
}

static void (*Meshserialize)(const Mesh *, Stream *);
INSERT_FUNCTION(0, Meshserialize);

static void (*Meshdeserialize)(Mesh *, Stream *);
INSERT_FUNCTION(1, Meshdeserialize);

void Mesh::serialize(Stream *stream) const {
    return Meshserialize(this, stream);
}

void Mesh::deserialize(Stream *stream) {
    return Meshdeserialize(this, stream);
}

void Mesh::clear() {
    m_vertices.clear();
    m_indices.clear();
    m_boneWeight.clear();
}
