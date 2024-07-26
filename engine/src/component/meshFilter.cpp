#include "meshFilter.hpp"

Mesh &MeshFilter::getMesh() {
    return m_mesh;
}

Boundbox &MeshFilter::getBoundbox() {
    return m_boundbox;
}
