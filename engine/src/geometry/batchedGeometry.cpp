#include "batchedGeometry.h"

#include "../component/MeshRender.h"
#include "../component/Transform.h"

#include "../gameObject.h"

#include "../resources/shader.h"

std::unordered_map<ulong, std::unique_ptr<BatchedGeometry>> BatchedGeometry::m_batchins;

BatchedGeometry::BatchedGeometry(const Mesh& mesh) : m_mesh(mesh), m_isCalculated(false), vertexCount(0) {
    m_vertexArray = grr::gVertexArray::Create();
    m_vertexArray->bind();

    m_indexEBO = grr::gVertexArray::CreateBuffer(grr::BufferType_EBO, mesh.getindicecount(), sizeof(u32));
    grr::gVertexArray::Bind(m_indexEBO);
    grr::gVertexArray::SetBufferUpdate(m_indexEBO, 0, mesh.getindicecount() * sizeof(u32), mesh.getindicedata());

    m_indexVBO = grr::gVertexArray::CreateBuffer(grr::BufferType_VBO, mesh.getverticecount(), sizeof(Vertex3D));
    grr::gVertexArray::Bind(m_indexVBO);
    grr::gVertexArray::SetBufferUpdate(m_indexVBO, 0, mesh.getverticecount() * sizeof(Vertex3D), mesh.getverticedata());

    grr::gVertexArray::SetAttrib(0, 3, sizeof(Vertex3D), reinterpret_cast<const void*>(offsetof(Vertex3D, position)));

    m_indexPrim = grr::gVertexArray::CreateBuffer(grr::BufferType_DEFAULT, 15000, sizeof(Matrix4x4));
    grr::gVertexArray::Bind(m_indexPrim);

    for (int i = 0; i < 4; ++i) {
        glEnableVertexAttribArray(1 + i);
        glVertexAttribPointer(1 + i, 4, GL_FLOAT, GL_FALSE, sizeof(Matrix4x4), (GLvoid*)(sizeof(Vector4) * i));
        glVertexAttribDivisor(1 + i, 1);
    }
    m_vertexArray->unbind();
}

BatchedGeometry::~BatchedGeometry() {
    if (m_indexEBO != -1) {
        grr::gVertexArray::DeleteBuffer(m_indexEBO);
    }
    if (m_indexVBO != -1) {
        grr::gVertexArray::DeleteBuffer(m_indexVBO);
    }
    if (m_indexPrim != -1) {
        grr::gVertexArray::DeleteBuffer(m_indexPrim);
    }
}

void BatchedGeometry::add(GameObject *gameobject) {
    m_objects.push_back(gameobject);
}

void BatchedGeometry::remove(GameObject *gameobject) {
    auto it = std::find(m_objects.begin(), m_objects.end(), gameobject);
    if (it != m_objects.end()) {
        m_objects.erase(it);
    }
}

void BatchedGeometry::render(const Frustum& frustum, Shader* shader) {
    // models.clear();

    m_vertexArray->bind();

    if (!m_isCalculated) {
        for (const auto& object : m_objects) {
            Transform& transform = object->getComponent<Transform>();
            MeshRender& meshRender = object->getComponent<MeshRender>();

            const Matrix4x4 model = transform.getglobalmatrix();

            // if (!frustum.IsCubeInFrustumOOBB(meshRender.m_boundBox.min, meshRender.m_boundBox.max, model)) {
            //     continue;
            // }

            models.push_back(model.transpose());
        }
        // m_isCalculated = true;
    }

    if (models.empty()) return;

    grr::gVertexArray::Bind(m_indexPrim);
    grr::gVertexArray::UpdateResize(m_indexPrim, models.size(), sizeof(Matrix4x4));
    grr::gVertexArray::SetBufferUpdate(m_indexPrim, 0, models.size() * sizeof(Matrix4x4), models.data());

    grr::gVertexArray::DrawElementsInstanced(grr::PrimitiveType_Triangles, m_mesh.getindicecount(), nullptr, models.size());
}

const std::vector<GameObject *> &BatchedGeometry::getObjects() const {
    return m_objects;
}



