#pragma once

#include "../common.h"

#include "../culling/frustum.h"

#include <vector>
#include <memory>
#include <array>

#define MAX_OCTREE_POINTS 8
#define MAX_OCTREE_CHILD 8
#define MAX_OCTREE_DEPTH 10

class Octree {
public:
    Octree(const Vector3& position, const Vector3& scale);

    void insert(const Vector3& position, GameObject* gameobject, u32 depth = 0);

    const bool contais(const Vector3& position) const;

    std::vector<GameObject*> render(const Frustum& frustum);

private:
    Vector3 m_position;

    Vector3 m_scale;

    bool m_subdivid;

    std::vector<GameObject*> m_points;

    std::array<std::unique_ptr<Octree>, MAX_OCTREE_CHILD> m_childrent;

    void subdivid();
};

