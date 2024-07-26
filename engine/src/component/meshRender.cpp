#include "meshRender.hpp"

#include "../scriptManager/scriptManager.hpp"

std::vector<GameObject *> &MeshRender::GetGameObjectFromRender() {
    return ScriptManager::GetGameObjectsFromRender();
}

std::unordered_map<int, Material> &MeshRender::getMaterials() {
    return m_materials;
}

void MeshRender::draw() {
    m_vertexArray->bind();

    grr::gVertexArray::DrawElements(grr::PrimitiveType_Triangles, m_count, nullptr);
}
