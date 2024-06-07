#pragma once

#include "../types.h"

#include <vector>
#include <memory>
#include <array>

#define MAX_OCTREE_POINTS 8
#define MAX_OCTREE_CHILD 8
#define MAX_OCTREE_DEPTH 10

class Octree;

class Octree {
public:
    Octree(const Vector3& position, const Vector3& scale);

    void insert(const Vector3& position, u32 depth = 0);

    const bool contais(const Vector3& position) const;

    void render();

private:
    Vector3 m_position;

    Vector3 m_scale;

    bool m_subdivid;

    std::vector<Vector3> m_points;

    std::array<std::unique_ptr<Octree>, MAX_OCTREE_CHILD> m_childrent;

    void subdivid();
};

