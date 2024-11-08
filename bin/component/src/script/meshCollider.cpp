#include "script/meshCollider.hpp"

#include "script/meshFilter.hpp"

#include "core/function.hpp"
#include "core/console.hpp"

MeshCollider::MeshCollider() : m_triangleMesh(nullptr), m_shape(nullptr) {}

void MeshCollider::Init() {
    if (!HasComponent<MeshFilter>()) {
        return;
    }
    auto meshFilter = GetComponent<MeshFilter>();
    
    try {
        build(meshFilter->getMesh());
    } catch (const std::exception &e) {
        Console::log(LogInfo::ERROR, "[ MeshCollider ] " + std::string(e.what()));
    }
}

static void (*MeshColliderbuild)(MeshCollider *, Mesh *);
INSERT_FUNCTION(0, MeshColliderbuild)

void MeshCollider::build(Mesh *mesh) {
    return MeshColliderbuild(this, mesh);
}





