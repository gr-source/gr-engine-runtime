#pragma once

#include "ecs/entity.h"

#include "geometry/vertex3D.h"
#include "default.hpp"

class MeshCollider : public Component {
public:
    MeshCollider() = delete;

    void build(Mesh *mesh);

    btCollisionShape *getShape();

private:
    std::shared_ptr<btTriangleMesh> m_triangleMesh;

    std::shared_ptr<btCollisionShape> m_shape;

    void buildWithIndice(const std::vector<int> &indices, const std::vector<Vertex3D> &vertices);

    void buildWithVertice(const std::vector<Vertex3D> &vertices);

    void setup(Transform *transform);
};



