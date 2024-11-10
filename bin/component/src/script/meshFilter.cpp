#include "script/meshFilter.hpp"

#include "core/assetsManager.hpp"
#include "core/threadpool.hpp"
#include "core/stream.hpp"
#include "core/event.hpp"

MeshFilter::MeshFilter() : m_sharedMesh(nullptr) { }

void MeshFilter::setMesh(Mesh *mesh) {
    m_sharedMesh = mesh;
}

Mesh *MeshFilter::getMesh() {
    return m_sharedMesh;
}

void MeshFilter::processBoundbox() {
    m_boundbox.min = {std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max()};
    m_boundbox.max = {std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), std::numeric_limits<float>::min()};
    
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
    if (!stream->write(reinterpret_cast<const void *>(&hasMesh), sizeof(bool))) {
        return false;
    }

    if (!hasMesh) {
        return true;
    }
    auto name = m_sharedMesh->getName();

    auto nSize = name.size();
    if (!stream->write(reinterpret_cast<const char *>(&nSize), sizeof(std::size_t))) {
        return false;
    }

    if (!stream->write(name.data(), nSize)) {
        return false;
    }

    m_sharedMesh->serialize(stream);

    return true;
}

void Setupmesh(Mesh *mesh, Mesh temp) {
    *mesh = temp;
}

void MeshFilterDeserializeMesh(std::streampos pos, std::string path, Mesh *mesh) {
    Stream stream(path, std::ios_base::in | std::ios_base::binary);
    stream.seekg(pos, std::ios_base::beg);

    Mesh tempMesh;
    tempMesh.deserialize(&stream);

    Event::AddEvent(&Setupmesh, mesh, tempMesh);
}

bool MeshFilter::Deserialize(Stream *stream) {
    bool hasMesh;
    if (!stream->read(reinterpret_cast<char *>(&hasMesh), sizeof(bool))) {
        return false;
    }

    if (!hasMesh) {
        return true;
    }

    std::size_t nsize = 0;
    if (!stream->read(reinterpret_cast<char *>(&nsize), sizeof(std::size_t))) {
        return false;
    }

    std::string name(nsize, '\0');
    if (!stream->read(name.data(), nsize)) {
        return false;
    }

    m_sharedMesh = AssetsManager::FindMesh(name);
    if (!m_sharedMesh) {
        m_sharedMesh = AssetsManager::CreateMesh(name, name);
        if (!m_sharedMesh) {
            return false;
        }
        // Threadpool::AddTask(&MeshFilterDeserializeMesh, stream->tellg(), stream->getPath(), m_sharedMesh);
        m_sharedMesh->deserialize(stream);
    }
    return true;
}







