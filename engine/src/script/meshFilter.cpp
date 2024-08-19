#include "meshFilter.hpp"

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
