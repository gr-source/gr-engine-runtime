#include "chunkManager.hpp"

#include "chunkController.h"
#include "camera.h"

#include "../scene/SceneManager.h"

#include "../eventManager.h"
#include "../input.h"

u8 ChunkManager::s_chunckSize = 16;

ChunkManager::ChunkManager() : Component(true) {
    // for (int i=0;i<9;i++) {
    //     m_chunks[i] = nullptr;
    // }
}

void ChunkManager::OnInit() {
    auto camera = Camera::GetCurrent();
    auto& position = camera->getPosition();

    int centerX = position.x / int(s_chunckSize);
    // int centerY = position.y / int(s_chunckSize);
    int centerZ = position.z / int(s_chunckSize);

    int loadDistance = 3;
    for (int dx = -loadDistance; dx <= loadDistance; ++dx) {
        for (int dz = -loadDistance; dz <= loadDistance; ++dz) {
            int chunkX = dx * s_chunckSize;
            int chunkZ = dz * s_chunckSize;

            std::cout << chunkX << " " << chunkZ << std::endl;
            
            // if (!ChunkIsLoaded(chunkX, chunkZ)) {
                EventManager::addEvent(&ChunkManager::LoadChunk, this, chunkX, chunkZ);
            // }
        }
    }
}

void ChunkManager::OnUpdate() {
    static float t = 0.0f;

    t += Time::deltaTime;

    if (t >= 1.0f) {
        // OnInit();
        t = 0.0f;
    }
}

void ChunkManager::LoadChunk(int x, int z) {
    // if (m_chunks.size() >= 9) return;

    auto chunk = SceneManager::add("Chunk[" + std::to_string(x) + "][" + std::to_string(z) + "]");
    auto& transform = chunk->getTransform();
    transform.position = {float(x), 0.0f, float(z)};

    chunk->addComponent<ChunkController>();

    m_chunks.push_back(chunk);

    // for (auto& slot : m_chunks) {
    //     if (slot) {
    //         continue;
    //     }
    //     slot = chunk;


        m_hasChunks.emplace(hash(x) ^ hash(z), chunk);
        // break;
    // }
}

const bool ChunkManager::ChunkIsLoaded(int x, int z) {
    auto it = m_hasChunks.find(hash(x) ^ hash(z));
    return it != m_hasChunks.end();
}
