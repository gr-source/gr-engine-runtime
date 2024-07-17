#include "chunkManager.hpp"

#include "chunkController.h"
#include "voxelSettings.hpp"
#include "camera.h"

#include "../scene/SceneManager.h"

#include "../eventManager.h"
#include "../input.h"


ChunkManager::ChunkManager() : Component(true) {}

void ChunkManager::OnInit() {
    auto camera = Camera::GetCurrent();
    auto& position = camera->getPosition();

    int centerX = position.x / int(VoxelSettings::ChunkSize);
    int centerZ = position.z / int(VoxelSettings::ChunkSize);

    int loadDistance = 1;
    for (int dx = -loadDistance; dx <= loadDistance; ++dx) {
        for (int dz = -loadDistance; dz <= loadDistance; ++dz) {
            int chunkX = dx * VoxelSettings::ChunkSize;
            int chunkZ = dz * VoxelSettings::ChunkSize;

            // if (!ChunkIsLoaded(chunkX, chunkZ)) {
                EventManager::addEvent(&ChunkManager::LoadChunk, this, chunkX, chunkZ);
            // }
        }
    }
}

void ChunkManager::OnUpdate() {
}

void ChunkManager::LoadChunk(int x, int z) {
    if (m_hasChunks[hash(x) ^ hash(z)]) {
        return;
    }
    
    auto chunk = SceneManager::add("Chunk[" + std::to_string(x) + "][" + std::to_string(z) + "]");
    
    auto& transform = chunk->getTransform();
    transform.position = {float(x), 0.0f, float(z)};

    chunk->addComponent<ChunkController>();

    m_chunks.push_back(chunk);

    m_hasChunks.emplace(hash(x) ^ hash(z), chunk);
}

GameObject *ChunkManager::findChunk(const Vector3 &position) {
    int x = ((static_cast<int>(position.x) - VoxelSettings::ChunkSize) / VoxelSettings::ChunkSize) * VoxelSettings::ChunkSize;
    int z = ((static_cast<int>(position.z) - VoxelSettings::ChunkSize) / VoxelSettings::ChunkSize) * VoxelSettings::ChunkSize;

    // std::cout << x << " " << z << std::endl;

    auto it = m_hasChunks.find(hash(x) ^ hash(z));
    return (it != m_hasChunks.end() ? (*it).second : nullptr);
}

const bool ChunkManager::ChunkIsLoaded(int x, int z) {
    auto it = m_hasChunks.find(hash(x) ^ hash(z));
    return it != m_hasChunks.end();
}
