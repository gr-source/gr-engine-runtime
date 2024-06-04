#include "batchedGeometry.h"

BatchedGeometry::BatchedGeometry(ulong size) : m_bufferManager(std::make_unique<BufferManager>(BufferManager::eSTATIC, true)) {
    m_bufferManager->bind(BufferManager::eVAO);

    m_bufferManager->bind(BufferManager::eEBO);
    m_bufferManager->pushBuffer(size * sizeof(u32), nullptr);

    m_bufferManager->bind(BufferManager::eVBO);
    m_bufferManager->pushBuffer(size * sizeof(Vertex3D), nullptr);

    m_bufferManager->unbind();
}