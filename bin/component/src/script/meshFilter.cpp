#include "script/meshFilter.hpp"

#include "core/assetsManager.hpp"
#include "core/stream.hpp"

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

bool MeshFilter::Serialize(Stream *stream) {
    bool hasMesh = m_sharedMesh != nullptr;
    stream->write(reinterpret_cast<const void *>(&hasMesh), sizeof(bool));

    if (!hasMesh) {
        return true;
    }
    auto &name = m_sharedMesh->getName();

    auto nSize = name.size();
    stream->write(reinterpret_cast<const char *>(&nSize), sizeof(std::size_t));
    stream->write(name.data(), nSize);
    return true;
}

bool MeshFilter::Deserialize(Stream *stream) {
    bool hasMesh;
    stream->read(reinterpret_cast<char *>(&hasMesh), sizeof(bool));

    if (!hasMesh) {
        return true;
    }

    std::size_t nSize;
    stream->read(reinterpret_cast<char *>(&nSize), sizeof(std::size_t));

    std::string name(nSize, '\0');
    stream->read(name.data(), nSize);

    m_sharedMesh = AssetsManager::FindMesh(name);

    return true;
}
