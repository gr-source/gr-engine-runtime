#include "meshFilter.h"

#include "../bufferManager.h"

MeshFilter::MeshFilter() : m_mesh("default") {}

Mesh &MeshFilter::getMesh() {
    return m_mesh;
}
