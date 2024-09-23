#include "script/meshFilter.hpp"

#include "core/assetsManager.hpp"

MeshFilter::MeshFilter() : m_sharedMesh(nullptr) { }

void MeshFilter::setMesh(Mesh *mesh) {
    m_sharedMesh = mesh;
}

Mesh *MeshFilter::getMesh() {
    return m_sharedMesh;
}

void MeshFilter::processBoundbox() {
    m_boundbox.min = Vector3(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
    m_boundbox.max = Vector3(std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), std::numeric_limits<float>::min());
    
    for (const auto& vertex : m_sharedMesh->getVertices()) {
        m_boundbox.min = Math::min(m_boundbox.min, vertex.position);
        m_boundbox.max = Math::max(m_boundbox.max, vertex.position);
    }
}

Boundbox &MeshFilter::getBoundbox() {
    return m_boundbox;
}

std::ostream &MeshFilter::Serialize(std::ostream &os) {
    bool hasMesh = m_sharedMesh != nullptr;
    os.write(reinterpret_cast<const char *>(&hasMesh), sizeof(bool));

    if (!hasMesh) {
        return os;
    }
    auto &name = m_sharedMesh->getName();

    auto nSize = name.size();
    os.write(reinterpret_cast<const char *>(&nSize), sizeof(std::size_t));
    os.write(name.data(), nSize);
    return os;
}

std::istream &MeshFilter::Deserialize(std::istream &is) {
    bool hasMesh;
    is.read(reinterpret_cast<char *>(&hasMesh), sizeof(bool));

    if (!hasMesh) {
        return is;
    }

    std::size_t nSize;
    is.read(reinterpret_cast<char *>(&nSize), sizeof(std::size_t));

    std::string name(nSize, '\0');
    is.read(name.data(), nSize);

    m_sharedMesh = AssetsManager::FindMesh(name);

    return is;
}
