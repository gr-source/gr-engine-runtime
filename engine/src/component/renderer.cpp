#include "renderer.h"

#include "meshFilter.h"

#include <gr-render/gVertexArray.h>

std::vector<GameObject*> Renderer::objects;

void Renderer::OnInit() {
    objects.push_back(entity->m_gameobject);

    buildMesh();
}

Renderer::Renderer() : m_buffer(std::make_unique<Buffer>()) {}

Renderer::~Renderer() {
    objects.erase(std::remove(objects.begin(), objects.end(), entity->m_gameobject));
}

void Renderer::buildMesh() {
    auto& meshFilter = entity->getComponent<MeshFilter>();

    auto& mesh = meshFilter.getMesh();

    m_buffer->buildMesh(mesh);
}

void Renderer::draw() {
    m_buffer->drawMesh();
}
