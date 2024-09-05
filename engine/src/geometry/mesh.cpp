#include "mesh.h"

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

void Mesh::serialize(std::ostream &os) const {
    auto &vertices = m_vertices;
    auto &indices = m_indices;
    auto &name = m_name;

    // Serialzie name
    auto nSize = name.size();
    os.write(reinterpret_cast<const char *>(&nSize), sizeof(std::size_t));

    os.write(name.data(), nSize);

    // Serialize vertices
    auto vSize = vertices.size();
    os.write(reinterpret_cast<const char *>(&vSize), sizeof(std::size_t));
    os.write(reinterpret_cast<const char *>(vertices.data()), vSize * sizeof(Vertex3D));

    // Serialize indices
    auto iSize = indices.size();
    os.write(reinterpret_cast<const char *>(&iSize), sizeof(std::size_t));
    os.write(reinterpret_cast<const char *>(indices.data()), iSize * sizeof(int));
}

void Mesh::deserialize(std::istream &is) {
    auto &vertices = m_vertices;
    auto &indices = m_indices;
    auto &name = m_name;

    // Deserialize name
    std::size_t nSize;
    is.read(reinterpret_cast<char *>(&nSize), sizeof(std::size_t));
    name.resize(nSize, '\0');
    is.read(name.data(), nSize);

    // Deserialize vertices
    std::size_t vSize;
    is.read(reinterpret_cast<char *>(&vSize), sizeof(std::size_t));
    vertices.resize(vSize);

    is.read(reinterpret_cast<char *>(vertices.data()), vSize * sizeof(Vertex3D));

    // Deserialize indices
    std::size_t iSize;
    is.read(reinterpret_cast<char *>(&iSize), sizeof(std::size_t));
    indices.resize(iSize);

    is.read(reinterpret_cast<char *>(indices.data()), iSize * sizeof(int));
}
