#include "script/meshCollider.hpp"

#include "script/meshFilter.hpp"

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

void MeshCollider::build(Mesh *mesh) {
    throw std::runtime_error("function do not implemented.");
}





