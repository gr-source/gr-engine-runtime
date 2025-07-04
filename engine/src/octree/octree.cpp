#include "octree.h"

#include <gr-base/ecs/gameObject.hpp>

#include <gr-base/script/meshRender.hpp>
#include <gr-base/script/transform.hpp>

#include "../renderer.h"
/*
Octree::Octree(const Vector3 &position, const Vector3& scale) : m_position(position), m_scale(scale), m_subdivid(false) {
    for (grm::u32 i=0;i<MAX_OCTREE_CHILD;i++) {
        m_childrent[i] = nullptr;
    }
}

void Octree::insert(const Vector3 &position, GameObject* gameobject, grm::u32 depth) {
    if (!contais(position) || depth > MAX_OCTREE_DEPTH) {
        return;
    }
    if (m_points.size() < MAX_OCTREE_POINTS) {
        m_points.push_back(gameobject);
        return;
    }
    
    if (!m_subdivid) {
        subdivid();
        m_subdivid = true;
    }

    for (auto& child : m_childrent) {
        child->insert(position, gameobject, depth + 1);
    }
}

const bool Octree::contais(const Vector3 &position) const {
    return (position.x > m_position.x - (m_scale.x / 2.0f) &&
            position.x < m_position.x + (m_scale.x / 2.0f) &&
            position.y > m_position.y - (m_scale.y / 2.0f) &&
            position.y < m_position.y + (m_scale.y / 2.0f) &&
            position.z > m_position.z - (m_scale.z / 2.0f) &&
            position.z < m_position.z + (m_scale.z / 2.0f)
        );
}

std::vector<GameObject*> Octree::render(const Frustum& frustum) {

    Vector3 vertices[8];
    vertices[0] = m_position + Vector3({-m_scale.x, -m_scale.y, -m_scale.z}) / 2.0f;
    vertices[1] = m_position + Vector3({ m_scale.x, -m_scale.y, -m_scale.z}) / 2.0f;
    vertices[2] = m_position + Vector3({ m_scale.x,  m_scale.y, -m_scale.z}) / 2.0f;
    vertices[3] = m_position + Vector3({-m_scale.x,  m_scale.y, -m_scale.z}) / 2.0f;
    vertices[4] = m_position + Vector3({-m_scale.x, -m_scale.y,  m_scale.z}) / 2.0f;
    vertices[5] = m_position + Vector3({ m_scale.x, -m_scale.y,  m_scale.z}) / 2.0f;
    vertices[6] = m_position + Vector3({ m_scale.x,  m_scale.y,  m_scale.z}) / 2.0f;
    vertices[7] = m_position + Vector3({-m_scale.x,  m_scale.y,  m_scale.z}) / 2.0f;

    // if (!frustum.IsCubeInFrustum(vertices)) {
    //     return std::vector<GameObject*>();
    // }

    std::vector<GameObject*> models;

    // for (const auto& point : m_points) {
    //     Transform& transform = point->getComponent<Transform>();
    //     MeshRender& meshRender = point->getComponent<MeshRender>();

    //     if (!frustum.IsCubeInFrustumOOBB(meshRender.m_boundBox.min, meshRender.m_boundBox.max, transform.getglobalmatrix())) {
    //         continue;
    //     }
    //     models.push_back(point);
    // }


    /* Desenha as linhas do cubo
    DebugDraw::addLine(vertices[0], vertices[1], m_points.empty() ? Color::black : Color::blue);
    DebugDraw::addLine(vertices[1], vertices[2], m_points.empty() ? Color::black : Color::blue);
    DebugDraw::addLine(vertices[2], vertices[3], m_points.empty() ? Color::black : Color::blue);
    DebugDraw::addLine(vertices[3], vertices[0], m_points.empty() ? Color::black : Color::blue);

    DebugDraw::addLine(vertices[4], vertices[5], m_points.empty() ? Color::black : Color::blue);
    DebugDraw::addLine(vertices[5], vertices[6], m_points.empty() ? Color::black : Color::blue);
    DebugDraw::addLine(vertices[6], vertices[7], m_points.empty() ? Color::black : Color::blue);
    DebugDraw::addLine(vertices[7], vertices[4], m_points.empty() ? Color::black : Color::blue);

    DebugDraw::addLine(vertices[0], vertices[4], m_points.empty() ? Color::black : Color::blue);
    DebugDraw::addLine(vertices[1], vertices[5], m_points.empty() ? Color::black : Color::blue);
    DebugDraw::addLine(vertices[2], vertices[6], m_points.empty() ? Color::black : Color::blue);
    DebugDraw::addLine(vertices[3], vertices[7], m_points.empty() ? Color::black : Color::blue);

    DebugDraw::addPoint(m_position, Color::blue);

    for (const auto& p : m_points) {
        DebugDraw::addPoint(p, Color::red);
    }

    for (auto& child : m_childrent) {
        if (!child) {
            continue;
        }
        auto childModel = child->render(frustum);

        models.insert(models.end(), childModel.begin(), childModel.end());
    }

    return models;
}

void Octree::subdivid() {
    Vector3 halfScale = m_scale / 2.0f;

    Vector3 centers[8];
    centers[0] = m_position + Vector3({-halfScale.x, -halfScale.y, -halfScale.z}) / 2.0f;
    centers[1] = m_position + Vector3({ halfScale.x, -halfScale.y, -halfScale.z}) / 2.0f;
    centers[2] = m_position + Vector3({ halfScale.x,  halfScale.y, -halfScale.z}) / 2.0f;
    centers[3] = m_position + Vector3({-halfScale.x,  halfScale.y, -halfScale.z}) / 2.0f;
    centers[4] = m_position + Vector3({-halfScale.x, -halfScale.y,  halfScale.z}) / 2.0f;
    centers[5] = m_position + Vector3({ halfScale.x, -halfScale.y,  halfScale.z}) / 2.0f;
    centers[6] = m_position + Vector3({ halfScale.x,  halfScale.y,  halfScale.z}) / 2.0f;
    centers[7] = m_position + Vector3({-halfScale.x,  halfScale.y,  halfScale.z}) / 2.0f;

    for (int i = 0; i < MAX_OCTREE_CHILD; ++i) {
        if (m_childrent[i] == nullptr)
            m_childrent[i] = std::make_unique<Octree>(centers[i], halfScale);
    }
}

*/
