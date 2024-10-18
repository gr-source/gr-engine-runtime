#pragma once

#include "default.hpp"

#include "../ecs/entity.hpp"

#include "core/geometry/batching.hpp"
#include "core/geometry/material.hpp"

#include <gr-render/gRender.h>

class MeshRender : public Component {
    CLASS_FUNCTIONS
public:
    MeshRender();
    ~MeshRender();

    void Init() override;

    template <typename ...Args>
    Material *addMaterial(Args &&...args);

    void build();

    void setMeshOptimize(int value);

    void Enable() override;

    void Disable() override;
    
    bool Serialize(Stream *stream) override;

    bool Deserialize(Stream *stream) override;

private:
    std::vector<std::shared_ptr<Material>> m_materials;

    int flags;

    Batching *m_batched;

    Transform *m_transform;
};

template <typename... Args>
inline Material *MeshRender::addMaterial(Args &&...args) {
    return &(*m_materials.emplace_back(std::make_shared<Material>(std::forward<Args>(args)...)));
}

REGISTER_COMPONENT(MeshRender);






