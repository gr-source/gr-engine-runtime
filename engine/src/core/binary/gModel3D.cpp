#include "gModel3D.hpp"

#include "gHeader.hpp"

#include "console.hpp"

#include "core/threadpool.hpp"

#include "core/filmbox/filmbox.hpp"

#include "core/file/stream.hpp"
#include "core/file/file.hpp"

void gModel3D::ImportFBX(const std::string &path) {
    Threadpool::AddTask(&Filmbox::Load, path, &gModel3D::SetupScene);
}

void gModel3D::SetupScene(std::shared_ptr<FbxScene> scene) {
    auto& fRootNode = scene->getRootNode();
    for (const auto& node : fRootNode.getChildren()) {
        WriteHeader(&(*scene), &node);
    }
}

void gModel3D::WriteHeader(const FbxScene *scene, const FbxNode *node) {
    LOG_INFO_ARGS("Write 3D Mode: %s", node->getName().c_str());

    Streaming stream(File::GetAsset("model/" + node->getName() + ".gmd"), std::ios_base::out | std::ios_base::binary);

    gHeader pHeader {{'g', 'r', 'm', 'd'}, 1.0f};
    if (!stream.writeS(&pHeader, 1, false)) {
        return LOG_ERROR("Failed to write the 3D model (0)");
    }

    WriteNode(scene, node, &stream);
}

void gModel3D::WriteNode(const FbxScene *scene, const FbxNode *node, Streaming *stream) {
    // Escreve os dados do modelo
    gModel pModel;
    pModel.m_scale = node->getScale();
    pModel.m_rotation = node->getRotation();
    pModel.m_position = node->getPosition();
    std::strcpy(pModel.m_name, node->getName().c_str());

    if (!stream->writeS(&pModel, 1, false)) {
        return;
    }

    auto &nodeMesh = node->getMeshes();

    // escreve a quantidade de mesh
    grm::u32 meshCount = static_cast<grm::u32>(nodeMesh.size());
    if (!stream->writeS(&meshCount, 1, false)) {
        std::cerr << "Failed to write mesh count to file." << std::endl;
        return;
    }

    // alloca as meshes na memória em forma de pilha (fácil acesso)
    std::vector<gMesh> meshes(meshCount);
    std::vector<gMaterial> pMateials;
    std::unordered_map<std::string, int> m_map;
    
    for (grm::u32 i = 0; i < meshCount; ++i) {
        auto &fbxMesh = nodeMesh[i];

        for (auto &mIdx : fbxMesh.getMaterials()) {
            auto& fbxMaterial = scene->getMaterial(mIdx);

            auto it = m_map.find(fbxMaterial.getName());
            if (it != m_map.end()) {
                continue;
            }
            m_map.emplace(fbxMaterial.getName(), 0);

            auto& m = pMateials.emplace_back();
            std::strcpy(m.m_name, fbxMaterial.getName().c_str());
            
            m.m_color = fbxMaterial.getColor();
            m.m_roughness = fbxMaterial.roughness;
            m.m_metallic = fbxMaterial.metallic;
            m.m_ambientOcclusion = fbxMaterial.ao;

            auto &textures = fbxMaterial.getTextures();

            m.m_textureCount = textures.size();
            m.m_textures = new gTexture[m.m_textureCount];

            int id = 0;
            for (auto &&[type, path] : textures) {
                auto &pTexture = m.m_textures[id];
                pTexture.m_type = gTextureType_albedo;
                std::strcpy(pTexture.m_path, path.c_str());
                id++;
            }
        }

        auto &vertices = fbxMesh.getVertices();
        auto &indices = fbxMesh.getIndices();

        auto &pMesh = meshes[i];
        std::memcpy(pMesh.m_name, fbxMesh.getName().c_str(), 256);

        pMesh.m_hasBone = fbxMesh.hasBone();
        
        pMesh.m_verticeCount = static_cast<grm::u32>(vertices.size());
        pMesh.m_vertices = new gVertex[pMesh.m_verticeCount];
        for (auto j=0;j<pMesh.m_verticeCount;++j) {
            pMesh.m_vertices[j].position = vertices[j].position;
            pMesh.m_vertices[j].normal   = vertices[j].normal;
            pMesh.m_vertices[j].texCoord = vertices[j].texCoord;
            pMesh.m_vertices[j].mIdx     = vertices[j].index;

            for (auto k=0;k<4;++k) {
                pMesh.m_vertices[j].weight[k]    = vertices[j].weight[k];
                pMesh.m_vertices[j].boneIndex[k] = vertices[j].boneIndex[k];
            }
        }

        pMesh.m_indicesCount = static_cast<grm::u32>(indices.size());
        pMesh.m_indices = new grm::u32[pMesh.m_indicesCount];
        for (int j=0;j<pMesh.m_indicesCount;++j) {
            pMesh.m_indices[j] = indices[j];
        }
        
        // Write bones from skeleton.
        if (!pMesh.m_hasBone) {
            continue;
        }

        auto &bones = fbxMesh.getBones();

        pMesh.m_boneCount = static_cast<grm::u32>(bones.size());
        pMesh.m_bones = new gBone[pMesh.m_boneCount];

        for (auto j=0;j<pMesh.m_boneCount;++j) {
            auto &fbxBone = bones[j];

            auto &name = fbxBone.getName();

            auto &pBone = pMesh.m_bones[j];
            std::strcpy(pBone.m_name, name.c_str());
            pBone.m_id = fbxBone.m_id;
            
            pBone.m_scale = fbxBone.getScale();
            pBone.m_rotation = fbxBone.getRotation();
            pBone.m_position = fbxBone.getPosition();
        }
    }

    // Escrever os dados das meshes no arquivo
    for (auto &mesh : meshes) {
        if (mesh.serialize(stream) != 0) {
            LOG_ERROR("Falied to write mesh: " + std::string(mesh.m_name));
            continue;
        }
    }

    grm::u32 mSize = pMateials.size();
    if (!stream->writeS(&mSize, 1, false)) {
        return LOG_ERROR("Falied to material data.");
    }

    for (auto &material : pMateials) {
        stream->writeS(material.m_name, 256, false);
        stream->writeS(&material.m_color, 1, false);
        stream->writeS(&material.m_roughness, 1, false);
        stream->writeS(&material.m_metallic, 1, false);
        stream->writeS(&material.m_ambientOcclusion, 1, false);

        stream->writeS(&material.m_textureCount, 1, false);
        stream->writeS(material.m_textures, material.m_textureCount, false);

        delete[] material.m_textures;
    }

    auto &children = node->getChildren();

    grm::u32 nSize = children.size();
    stream->writeS(&nSize, 1, false);

    // escreve os filhos
    for (const auto &child : children) {
        WriteNode(scene, &child, stream);
    }
}
