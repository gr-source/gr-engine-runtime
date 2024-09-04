#pragma once

#include "../ecs/entity.h"

#include "../geometry/material.h"

#include <gr-render/gVertexArray.h>
#include <gr-render/gRender.h>

#include "default.hpp"

class MeshRender : public Component {
public:
    MeshRender() = delete;

    std::vector<Material> &getMaterials();

    template <typename ...Args>
    Material &addMaterial(Args &&...args);

    void build();

private:
    std::vector<Material> m_materials;

    Batching *m_batched;
};

template <typename... Args>
inline Material &MeshRender::addMaterial(Args &&...args) {
    return m_materials.emplace_back(std::forward<Args>(args)...);
}
