#pragma once

#include "../ecs/entity.h"

#include "../geometry/material.h"

#include <gr-render/gVertexArray.h>
#include <gr-render/gRender.h>

#include "default.hpp"

class MeshRender : public Component {
public:
    MeshRender() = delete;

    std::unordered_map<int, Material> &getMaterials();

    Material &addMaterial(int idx, const Material& material);

    void build();

private:
    std::unordered_map<int, Material> m_materials;

    Batching *m_batched;
};



