#pragma once

#include <ecs/entity.hpp>

#include "../script/default.hpp"

class MeshCollider : public Component {
    CLASS_FUNCTIONS
public:
    MeshCollider();

    void Init() override;

    void build(Mesh *mesh);
    
private:
    std::shared_ptr<btTriangleMesh> m_triangleMesh;

    std::shared_ptr<btCollisionShape> m_shape;
};

REGISTER_COMPONENT(MeshCollider)



