#pragma once

#include "../ecs/entity.h"

#include "../common.h"

struct Vector2Hash {
    std::size_t operator()(const Vector2& v) const {
        return std::hash<float>()(v.x) ^ (std::hash<float>()(v.y) << 1);
    }
};

class ChunkManager : public Component {
public:
    ChunkManager();

    void OnInit() override;

    void OnUpdate() override;

    const bool ChunkIsLoaded(int x, int z);

    void LoadChunk(int x, int z);
private:
    std::vector<GameObject*> m_chunks;

    std::unordered_map<ulong, bool> m_hasChunks;

    std::hash<int> hash;

    static u8 s_chunckSize;
};

