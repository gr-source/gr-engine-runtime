#pragma once

#include "../common.h"
#include "../ecs/entity.h"
#include "../simplexNoise/SimplexNoise.h"

#include "block.h"

class ChunkController : public Component {
public:
    enum FaceRender {
        FaceRender_Top    = 1,
        FaceRender_Bottom = 2,
        FaceRender_Front  = 3,
        FaceRender_Back   = 4,
        FaceRender_Right  = 5,
        FaceRender_Left   = 6
    };

    ChunkController();

    void OnInit() override;

    void BuildMesh();

    std::unordered_map<FaceRender, bool> face;

private:
    static SimplexNoise Noise;

    static int ChunkSize;

    std::array<std::array<std::array<Block, 16>, 16>, 16> m_blocks;

    const bool checkFace(const Vector3& position);

    friend class Game;
};



