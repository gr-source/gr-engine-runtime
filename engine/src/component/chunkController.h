#pragma once

#include "../common.h"
#include "../ecs/entity.h"
#include "../simplexNoise/SimplexNoise.h"

#include "block.h"

class ChunkController : public Component {
public:

    ChunkController();
    ~ChunkController();

    void OnInit() override;

    void BuildMesh();

    const Vector3& getPosition() const;

private:
    static SimplexNoise Noise;

    static std::vector<ChunkController*> m_chunks;

    std::array<std::array<std::array<Block, 16>, 16>, 16> m_blocks;

    const bool checkFace(const Vector3& position);

    static ChunkController* findChunk(const Vector3& postion);

    friend class Game;
};
