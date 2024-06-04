#include "batchedGeometry.h"

std::unordered_map<ulong, std::unique_ptr<BatchedGeometry>> BatchedGeometry::m_batchins;

BatchedGeometry::BatchedGeometry(ulong size) : m_bufferManager(std::make_unique<BufferManager>(BufferManager::eSTATIC, true)) {
    m_bufferManager->bind(BufferManager::eVAO);

    m_bufferManager->bind(BufferManager::eEBO);
    m_bufferManager->pushElements(size * sizeof(u32), nullptr);

    m_bufferManager->bind(BufferManager::eVBO);
    m_bufferManager->pushBuffer(size * sizeof(Vertex3D), nullptr);

    m_bufferManager->attribPointer(0, 3, sizeof(Vertex3D), reinterpret_cast<void*>(offsetof(Vertex3D, position)));
    // m_bufferManager->attribPointer(1, 3, sizeof(Vertex3D), reinterpret_cast<void*>(offsetof(Vertex3D, normal)));
    // m_bufferManager->attribPointer(2, 2, sizeof(Vertex3D), reinterpret_cast<void*>(offsetof(Vertex3D, uv)));
    // m_bufferManager->attribPointer(3, 2, sizeof(Vertex3D), reinterpret_cast<void*>(offsetof(Vertex3D, tangent)));
    // m_bufferManager->attribPointer(4, 2, sizeof(Vertex3D), reinterpret_cast<void*>(offsetof(Vertex3D, bitangent)));
    // m_bufferManager->attribPointer(5, 1, sizeof(Vertex3D), reinterpret_cast<void*>(offsetof(Vertex3D, materialIndex)));
    // m_bufferManager->attribPointer(6, 4, GL_INT, sizeof(Vertex3D), reinterpret_cast<void*>(offsetof(Vertex3D, boneIndex)));
    // m_bufferManager->attribPointer(7, 4, sizeof(Vertex3D), reinterpret_cast<void*>(offsetof(Vertex3D, weight)));

    m_bufferManager->unbind();
}

void BatchedGeometry::add(GameObject *gameobject) {
    m_objects.push_back(gameobject);
}

void BatchedGeometry::remove(GameObject *gameobject) {
    auto it = std::find(m_objects.begin(), m_objects.end(), gameobject);
    if (it != m_objects.end()) {
        m_objects.erase(it);
    }
}

const std::vector<GameObject *> &BatchedGeometry::getObjects() const {
    return m_objects;
}

BufferManager *BatchedGeometry::getBufferManager() {
    return m_bufferManager.get();
}
