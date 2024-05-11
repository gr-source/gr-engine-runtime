#include "MeshRender.h"

#include "cameraComponent.h"
#include "Transform.h"

#include "../bufferManager.h"
#include "../screen.h"

#include "../resources/Resources.h"

#include "../ecs/Entity.h"

#include "../ui/View.h"

MeshRender::MeshRender(const Mesh& mesh, const std::unordered_map<u32, Material>& materials) : Component(true), materials(materials), verticeCount(0), indiceCount(0), drawcount(0), m_occlusion(std::make_unique<Occlusion>()) {
}

MeshRender::MeshRender(const std::vector<Mesh>& meshes, const std::unordered_map<u32, Material>& materials) : Component(true), materials(materials), verticeCount(0), indiceCount(0), drawcount(0), m_occlusion(std::make_unique<Occlusion>()) {
    std::vector<Vertex3D> m_vertices;
    std::vector<GLuint> m_indices;

    for (const auto& mesh : meshes) {
        auto& vertices = mesh.getvertices();
        auto& indices = mesh.getindices();

        m_vertices.insert(m_vertices.end(), vertices.begin(), vertices.end());

        for (GLuint index : indices) {
            m_indices.push_back(index + verticeCount);
        }
        verticeCount += vertices.size();
    }

    Vector3 minBound = Vector3(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
    Vector3 maxBound = Vector3(std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), std::numeric_limits<float>::min());
    
    for (const auto& vertex : m_vertices) {
        // Atualize os valores mínimos e máximos para cada dimensão
        minBound.x = std::min(minBound.x, vertex.position.x);
        minBound.y = std::min(minBound.y, vertex.position.y);
        minBound.z = std::min(minBound.z, vertex.position.z);

        maxBound.x = std::max(maxBound.x, vertex.position.x);
        maxBound.y = std::max(maxBound.y, vertex.position.y);
        maxBound.z = std::max(maxBound.z, vertex.position.z);
    }

    m_drawcall.m_boundBox.min = minBound;
    m_drawcall.m_boundBox.max = maxBound;

    indiceCount = m_indices.size();

    // Configura os buffers
    m_drawcall.buffers.setup(BufferManager::eSTATIC, true);
    m_drawcall.buffers.bind(BufferManager::eVAO);

    m_drawcall.buffers.bind(BufferManager::eEBO);
    m_drawcall.buffers.pushElements(indiceCount * sizeof(GLuint), m_indices.data());

    m_drawcall.buffers.bind(BufferManager::eVBO);
    m_drawcall.buffers.pushBuffer(verticeCount * sizeof(Vertex3D), m_vertices.data());

    /* configure o buffer do shader */
    AddAttrib(m_drawcall.buffers);

    m_drawcall.buffers.unbind();

    gameStatics.vertices += verticeCount;
    gameStatics.indices += indiceCount;

    BatchingManager::drawCount += 1;
}

void MeshRender::OnInit() {
    m_transform = &entity->getComponent<Transform>();
}

void MeshRender::OnGui() {
    if (ImGui::TreeNode("MeshRender")) {
        for (auto& entry : materials) {
            Material& material = entry.second;
            
            if (ImGui::TreeNode(std::string(material.name + "(" + std::to_string(entry.first) + ")").c_str())) {
                {
                    ImGui::SeparatorText("Albedo");
                    if (material.texture[TexturePBR::ALBEDO] != nullptr && material.texture[TexturePBR::ALBEDO]->isvalid()) {
                        ImGui::Image(reinterpret_cast<ImTextureID>(material.texture[TexturePBR::ALBEDO]->getID()), ImVec2(32, 32));
                        if (ImGui::IsItemHovered() && ImGui::IsKeyDown(ImGuiKey_X)) {
                            material.texture[TexturePBR::ALBEDO] = nullptr;
                        }
                    } else {
                        if (View::TextureInputUI(ImColor(material.albedo.r, material.albedo.g, material.albedo.b, material.albedo.a))) {
                            ImGui::OpenPopup("#PopupChangeColorAlbedo");
                        }
                    }

                    if (ImGui::BeginDragDropTarget()) {
                        if (const ImGuiPayload* data = ImGui::AcceptDragDropPayload(getDragDrop(DragDrop::TEXTURE), ImGuiDragDropFlags_None)) {
                            std::string path((char*)data->Data, data->DataSize);
                            if (!path.empty()) {
                                material.texture[TexturePBR::ALBEDO] = Resources::Load<Texture2D>(path);
                            }
                        }
                        ImGui::EndDragDropTarget();
                    }
                }

                {
                    ImGui::SeparatorText("Normal");
                    if (material.texture[TexturePBR::NORMAL] != nullptr && material.texture[TexturePBR::NORMAL]->isvalid()) {
                        ImGui::Image(reinterpret_cast<ImTextureID>(material.texture[TexturePBR::NORMAL]->getID()), ImVec2(32, 32));
                        if (ImGui::IsItemHovered() && ImGui::IsKeyDown(ImGuiKey_X)) {
                            material.texture[TexturePBR::NORMAL] = nullptr;
                        }
                    } else {
                        View::TextureInputUI(ImColor(100, 100, 255));
                    }

                    if (ImGui::BeginDragDropTarget()) {
                        if (const ImGuiPayload* data = ImGui::AcceptDragDropPayload(getDragDrop(DragDrop::TEXTURE), ImGuiDragDropFlags_None)) {
                            std::string path((char*)data->Data, data->DataSize);
                            if (!path.empty()) {
                                material.texture[TexturePBR::NORMAL] = Resources::Load<Texture2D>(path);
                            }
                        }
                        ImGui::EndDragDropTarget();
                    }
                }

                {
                    ImGui::SeparatorText("Metalica");
                    if (material.texture[TexturePBR::METALLIC] != nullptr && material.texture[TexturePBR::METALLIC]->isvalid()) {
                        ImGui::Image(reinterpret_cast<ImTextureID>(material.texture[TexturePBR::METALLIC]->getID()), ImVec2(32, 32));
                        if (ImGui::IsItemHovered() && ImGui::IsKeyDown(ImGuiKey_X)) {
                            material.texture[TexturePBR::METALLIC] = nullptr;
                        }
                    } else {
                        ImGui::DragFloat("##Metalica", &material.metallic, 0.005f, 0.0f, 1.0f);
                    }

                    if (ImGui::BeginDragDropTarget()) {
                        if (const ImGuiPayload* data = ImGui::AcceptDragDropPayload(getDragDrop(DragDrop::TEXTURE), ImGuiDragDropFlags_None)) {
                            std::string path((char*)data->Data, data->DataSize);
                            if (!path.empty()) {
                                material.texture[TexturePBR::METALLIC] = Resources::Load<Texture2D>(path);
                            }
                        }
                        ImGui::EndDragDropTarget();
                    }
                }

                {
                    ImGui::SeparatorText("Rugosidade");
                    if (material.texture[TexturePBR::ROUGHNESS] != nullptr && material.texture[TexturePBR::ROUGHNESS]->isvalid()) {
                        ImGui::Image(reinterpret_cast<ImTextureID>(material.texture[TexturePBR::ROUGHNESS]->getID()), ImVec2(32, 32));
                        if (ImGui::IsItemHovered() && ImGui::IsKeyDown(ImGuiKey_X)) {
                            material.texture[TexturePBR::ROUGHNESS] = nullptr;
                        }
                    } else {
                        ImGui::DragFloat("##Rugosidade", &material.roughness, 0.005f, 0.0f, 1.0);
                    }

                    if (ImGui::BeginDragDropTarget()) {
                        if (const ImGuiPayload* data = ImGui::AcceptDragDropPayload(getDragDrop(DragDrop::TEXTURE), ImGuiDragDropFlags_None)) {
                            std::string path((char*)data->Data, data->DataSize);
                            if (!path.empty()) {
                                material.texture[TexturePBR::ROUGHNESS] = Resources::Load<Texture2D>(path);
                            }
                        }
                        ImGui::EndDragDropTarget();
                    }
                }

                {
                    ImGui::SeparatorText("AO");
                    if (material.texture[TexturePBR::AO] != nullptr && material.texture[TexturePBR::AO]->isvalid()) {
                        ImGui::Image(reinterpret_cast<ImTextureID>(material.texture[TexturePBR::AO]->getID()), ImVec2(32, 32));
                        if (ImGui::IsItemHovered() && ImGui::IsKeyDown(ImGuiKey_X)) {
                            material.texture[TexturePBR::AO] = nullptr;
                        }
                    } else {
                        ImGui::DragFloat("##AO", &material.ao, 0.005f, 0.0f, 1.0);
                    }

                    if (ImGui::BeginDragDropTarget()) {
                        if (const ImGuiPayload* data = ImGui::AcceptDragDropPayload(getDragDrop(DragDrop::TEXTURE), ImGuiDragDropFlags_None)) {
                            std::string path((char*)data->Data, data->DataSize);
                            if (!path.empty()) {
                                material.texture[TexturePBR::AO] = Resources::Load<Texture2D>(path);
                            }
                        }
                        ImGui::EndDragDropTarget();
                    }
                }

                View::PopupChangeColor("Albedo", material.albedo);

                ImGui::TreePop();
            }
        }
        ImGui::TreePop();
    }
}

void MeshRender::AddAttrib(BufferManager& pBufferManager) {
    /* posiçao do vertice no espaço mundial */
    pBufferManager.attribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex3D), reinterpret_cast<void*>(offsetof(Vertex3D, position)));

    /* normal do objeto noe spaço mundial */
    pBufferManager.attribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex3D), reinterpret_cast<void*>(offsetof(Vertex3D, normal)));

    /* uv usada para desenhar textura */
    pBufferManager.attribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex3D), reinterpret_cast<void*>(offsetof(Vertex3D, uv)));

    /* tangente do objeto espaço mundial */
    pBufferManager.attribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex3D), reinterpret_cast<void*>(offsetof(Vertex3D, tangent)));

    /* bitangent usado pra luz espaço mundial */
    pBufferManager.attribPointer(4, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex3D), reinterpret_cast<void*>(offsetof(Vertex3D, bitangent)));

    /* id do material attribuido a vertice */
    pBufferManager.attribPointer(5, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex3D), reinterpret_cast<void*>(offsetof(Vertex3D, materialIndex)));
    
    /* id do bone */
    pBufferManager.attribPointer(6, 4, GL_INT, sizeof(Vertex3D), reinterpret_cast<void*>(offsetof(Vertex3D, boneIndex)));

    /* peso do bone attibuido ao vertice */
    pBufferManager.attribPointer(7, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex3D), reinterpret_cast<void*>(offsetof(Vertex3D, weight)));
}

MeshRender::~MeshRender() {
    gameStatics.vertices -= verticeCount;
    gameStatics.indices  -= indiceCount;

    BatchingManager::drawCount -= 1;
}


