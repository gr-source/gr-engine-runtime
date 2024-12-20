#include "meshCollider.hpp"

#include "script/transform.hpp"

#include "physics/physicsScene.hpp"

#include <core/geometry/mesh.hpp>

#include "core/console.hpp"

#include "ecs/entity.hpp"

void MeshCollider::build(Mesh *mesh) {
    if (!mesh) {
        return Console::LogError("[ MeshCollider ] Game object do not has mesh.");
    }

    m_triangleMesh = std::make_shared<btTriangleMesh>();

    auto indices = mesh->getIndices();
    auto vertices = mesh->getVertices();

    if (vertices.empty()) {
        throw std::runtime_error("Mesh do not has vertices");
    }

    if (!indices.empty()) {
        return buildWithIndice(indices, vertices);
    }
    buildWithVertice(vertices);
}

btCollisionShape *MeshCollider::getShape() {
    return &(*m_shape);
}

void MeshCollider::buildWithIndice(const std::vector<int> &indices, const std::vector<Vertex3D> &vertices) {
    for (grm::u32 i = 0; i < indices.size(); i += 3) {
        if (i + 2 >= indices.size()) {
            return Console::LogError("[ MeshCollider ] Index access out of bounds.");
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
    return setup(entity->getComponent<Transform>());
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

    return setup(entity->getComponent<Transform>());
}

void MeshCollider::setup(Transform *transform) {
    m_shape = std::make_shared<btBvhTriangleMeshShape>(m_triangleMesh.get(), true);
    m_shape->setLocalScaling(btVector3(transform->getLocalScale().x, transform->getLocalScale().y, transform->getLocalScale().z));
}
