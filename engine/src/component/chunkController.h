#pragma once

#include "../common.h"
#include "../ecs/entity.h"
#include "../simplexNoise/SimplexNoise.h"

#include "block.h"

class ChunkController : public Component {
public:
    ChunkController();

    void OnInit() override;

    void BuildMesh();

private:
    static SimplexNoise Noise;
    static int ChunkSize;

    std::array<std::array<std::array<Block, 16>, 16>, 16> m_blocks;

    friend class Game;
};



