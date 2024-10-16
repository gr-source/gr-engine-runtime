#include "gHeader.hpp"

#include "core/file/stream.hpp"

gMesh::gMesh() : m_hasBone(false), m_vertices(nullptr), m_verticeCount(0), m_indices(nullptr), m_indicesCount(0), m_bones(nullptr), m_boneCount(0) {}

gMesh::~gMesh() {
    if (m_vertices) {
        delete []m_vertices;
    }
    if (m_indices) {
        delete []m_indices;
    }
    if (m_bones && m_hasBone) {
        delete []m_bones;
    }
}

int gMesh::serialize(grm::size *offset) {
    if (offset) {
        *offset += sizeof(char)     * 256;
        *offset += sizeof(bool);
        *offset += sizeof(gVertex)  * m_verticeCount;
        *offset += sizeof(grm::u32) * m_indicesCount;
        *offset += sizeof(gBone)    * m_boneCount;
    }

    // metadata
    if (!Stream::write(m_name, 256)) {
        return 1;
    }
    if (!Stream::write(&m_hasBone, 1)) {
        return 2;
    }

    // data
    if (!Stream::write(&m_verticeCount, 1)) {
        return 3;
    }
    if (!Stream::write(m_vertices, m_verticeCount)) {
        return 4;
    }
    
    if (!Stream::write(&m_indicesCount, 1)) {
        return 5;
    }
    if (!Stream::write(m_indices, m_indicesCount)) {
        return 6;
    }

    if (m_hasBone) {
        if (!Stream::write(&m_boneCount, 1)) {
            return 7;
        }
        if (!Stream::write(m_bones, m_boneCount)) {
            return 8;
        }
    }
    return 0;
}

int gMesh::deserialize() {
    // metadata
    if (!Stream::read(m_name, 256)) {
        return 1;
    }
    if (!Stream::read(&m_hasBone, 1)) {
        return 2;
    }

    // data
    // vertice
    {
        if (!Stream::read(&m_verticeCount, 1)) {
            return 3;
        }
        if (m_vertices) {
            delete []m_vertices;
        }
        m_vertices = new gVertex[m_verticeCount];
        
        if (!Stream::read(m_vertices, m_verticeCount)) {
            return 4;
        }
    }
    
    // indices
    {
        if (!Stream::read(&m_indicesCount, 1)) {
            return 5;
        }
        if (m_indices) {
            delete []m_indices;
        }
        m_indices = new grm::u32[m_indicesCount];
        if (!Stream::read(m_indices, m_indicesCount)) {
            return 6;
        }
    }

    // bone
    if (m_hasBone) {
        if (!Stream::read(&m_boneCount, 1)) {
            return 7;
        }
        if (m_bones) {
            delete []m_bones;
        }
        m_bones = new gBone[m_boneCount];
        if (!Stream::read(m_bones, m_boneCount)) {
            return 8;
        }
    }
    return 0;
}
