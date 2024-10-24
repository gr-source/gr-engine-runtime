#include "mesh.h"

#include "core/file/stream.hpp"

Mesh::Mesh(const std::string &name) : m_name(name) {}

void Mesh::setName(const std::string &name) {
    m_name = name;
}

const std::string &Mesh::getName() const {
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

std::vector<int> &Mesh::getIndices() {
    return m_indices;
}

const bool Mesh::hasIndices() const {
    return !m_indices.empty();
}

const std::vector<BoneWeight> &Mesh::getBoneWeight() const {
    return m_boneWeight;
}

void Mesh::serialize(Streaming *streaming) const {
    auto &vertices = m_vertices;
    auto &indices = m_indices;
    auto &name = m_name;

    // Serialzie name
    auto nSize = name.size();
    streaming->writeS(&nSize, 1);

    streaming->writeS(name.data(), nSize);

    // Serialize vertices
    auto vSize = vertices.size();
    streaming->writeS(&vSize, 1);
    streaming->writeS(vertices.data(), vSize);

    // Serialize indices
    auto iSize = indices.size();
    streaming->writeS(&iSize, 1);
    streaming->writeS(indices.data(), iSize);
}

void Mesh::deserialize(Streaming *streaming) {
    auto &vertices = m_vertices;
    auto &indices = m_indices;
    auto &name = m_name;

    // Deserialize name
    std::size_t nSize;
    streaming->readS(&nSize, 1);
    name.resize(nSize, '\0');
    streaming->readS(name.data(), nSize);

    // Deserialize vertices
    std::size_t vSize;
    streaming->readS(&vSize, 1);
    vertices.resize(vSize);

    streaming->readS(vertices.data(), vSize);

    // Deserialize indices
    std::size_t iSize;
    streaming->readS(&iSize, 1);
    indices.resize(iSize);

    streaming->readS(indices.data(), iSize);
}

