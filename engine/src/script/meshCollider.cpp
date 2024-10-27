#include "meshCollider.hpp"

#include "script/transform.hpp"

#include "physics/physicsScene.hpp"

#include "geometry/mesh.h"

#include "core/console.hpp"

void MeshCollider::build(Mesh *mesh) {
    if (!mesh) {
        return LOG_ERROR("[ MeshCollider ] Game object do not has mesh.");
    }

    auto &transform = entity->getComponent<Transform>();
    m_triangleMesh = std::make_shared<btTriangleMesh>();

    const auto &indices = mesh->getIndices();
    const auto &vertices = mesh->getVertices();

    if (indices.empty() && vertices.empty()) {
        return LOG_ERROR("[ MeshCollider ] Mesh has no data.");
    } else if (!vertices.empty()) {
        buildWithVertice(vertices);

        return setup(&transform);
    }

    if (indices.size() % 3 != 0) {
        return LOG_ERROR("[ MeshCollider ] Indices size is not a multiple of 3.");
    }

    buildWithIndice(indices, vertices);

    if (!m_triangleMesh) {
        return LOG_ERROR("[ MeshCollider ] triangle ptr invalid.");
    }

    setup(&transform);
}

btCollisionShape *MeshCollider::getShape() {
    return &(*m_shape);
}

void MeshCollider::buildWithIndice(const std::vector<int> &indices, const std::vector<Vertex3D> &vertices) {
    for (grm::u32 i = 0; i < indices.size(); i += 3) {
        if (i + 2 >= indices.size()) {
            return LOG_ERROR("[ MeshCollider ] Index access out of bounds.");
        }

        auto &vertex1 = vertices[indices[i]];
        auto &vertex2 = vertices[indices[i + 1]];
        auto &vertex3 = vertices[indices[i + 2]];

        auto trangle1 = vertex1.position;
        auto trangle2 = vertex2.position;
        auto trangle3 = vertex3.position;

        m_triangleMesh->addTriangle(
            btVector3(trangle1.x, trangle1.y, trangle1.z),
            btVector3(trangle2.x, trangle2.y, trangle2.z),
            btVector3(trangle3.x, trangle3.y, trangle3.z)
        );
    }
}

void MeshCollider::buildWithVertice(const std::vector<Vertex3D> &vertices) {
    for (size_t i = 0; i < vertices.size(); i += 3) {
        if (i + 2 >= vertices.size()) {
            throw std::runtime_error("Vertex access out of bounds.");
        }

        auto &vertex1 = vertices[i];
        auto &vertex2 = vertices[i + 1];
        auto &vertex3 = vertices[i + 2];

        auto trangle1 = vertex1.position;
        auto trangle2 = vertex2.position;
        auto trangle3 = vertex3.position;

        m_triangleMesh->addTriangle(
            btVector3(trangle1.x, trangle1.y, trangle1.z),
            btVector3(trangle2.x, trangle2.y, trangle2.z),
            btVector3(trangle3.x, trangle3.y, trangle3.z)
        );
    }
}

void MeshCollider::setup(Transform *transform) {
    m_shape = std::make_shared<btBvhTriangleMeshShape>(m_triangleMesh.get(), true);
    m_shape->setLocalScaling(btVector3(transform->getLocalScale().x, transform->getLocalScale().y, transform->getLocalScale().z));
}
