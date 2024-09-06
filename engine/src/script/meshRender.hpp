#pragma once

#include "../ecs/entity.h"

#include "geometry/batching.hpp"
#include "geometry/material.h"

#include "default.hpp"

class MeshRender : public Component {
public:
    MeshRender() = delete;

    std::vector<Material> &getMaterials();

    template <typename ...Args>
    Material &addMaterial(Args &&...args);

    void build();

    Batching *getBatchGroup();

    void setMeshOptimize(int value);

private:
    std::vector<Material> m_materials;

    int flags;

    Batching *m_batched;

    friend class Game;
};

template <typename... Args>
inline Material &MeshRender::addMaterial(Args &&...args) {
    return m_materials.emplace_back(std::forward<Args>(args)...);
}
