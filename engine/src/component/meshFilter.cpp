#include "meshFilter.hpp"

Mesh &MeshFilter::getMesh() {
    return m_mesh;
}

void MeshFilter::processBoundbox() {
    m_boundbox.min = Vector3(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
    m_boundbox.max = Vector3(std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), std::numeric_limits<float>::min());
    
    for (const auto& vertex : m_mesh.getVertices()) {
        m_boundbox.min.x = std::min(m_boundbox.min.x, vertex.position.x);
        m_boundbox.min.y = std::min(m_boundbox.min.y, vertex.position.y);
        m_boundbox.min.z = std::min(m_boundbox.min.z, vertex.position.z);

        m_boundbox.max.x = std::max(m_boundbox.max.x, vertex.position.x);
        m_boundbox.max.y = std::max(m_boundbox.max.y, vertex.position.y);
        m_boundbox.max.z = std::max(m_boundbox.max.z, vertex.position.z);
    }
}

Boundbox &MeshFilter::getBoundbox() {
    return m_boundbox;
}
