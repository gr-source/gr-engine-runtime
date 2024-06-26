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

    std::array<std::array<std::array<Block, 16>, 16>, 16> m_blocks;

    const bool checkFace(const Vector3& position);

    friend class Game;
};



