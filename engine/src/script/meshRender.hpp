#pragma once


#include "geometry/batching.hpp"
#include "geometry/material.h"

#include "default.hpp"

#include "ecs/component.hpp"

class MeshRender : public Component {
public:
    MeshRender() = delete;

    std::vector<std::shared_ptr<Material>> &getMaterials();

    template <typename ...Args>
    Material *addMaterial(Args &&...args);

    void build();

    Batching *getBatchGroup();

    void setMeshOptimize(int value);

    const int &getFlags() const;

private:
    std::vector<std::shared_ptr<Material>> m_materials;

    int m_flags;

    Batching *m_batched;

    Transform *m_transform;

    void SetupShader(Mesh *mesh, grr::gShader *shader);
};

template <typename... Args>
inline Material *MeshRender::addMaterial(Args &&...args) {
    return &(*m_materials.emplace_back(std::make_shared<Material>(std::forward<Args>(args)...)));
}
