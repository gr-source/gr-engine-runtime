#pragma once

#include "collider.h"

#include "../geometry/mesh.h"

#include "../ecs/Entity.h"

class MeshCollider : public Component {
public:
    MeshCollider(const Mesh& mesh);

    void OnInit() override;

    void OnUpdate() override;

    void OnGui() override;
protected:
    Transform* m_transform;
    
    btTriangleMeshShape* m_TriangleMeshShape;

    std::unique_ptr<btTriangleMesh> m_triangleMesh;
};

