#pragma once

#include "../ecs/entity.h"

#include "../common.h"

class ChunkManager : public Component {
public:
    ChunkManager();

    void OnInit() override;

    void OnUpdate() override;

    const bool ChunkIsLoaded(int x, int z);

    void LoadChunk(int x, int z);
private:
    std::vector<GameObject*> m_chunks;

    std::unordered_map<ulong, GameObject*> m_hasChunks;

    std::hash<int> hash;

    GameObject* findChunk(const Vector3& position);
};

