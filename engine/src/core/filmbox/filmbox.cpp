#include "filmbox.hpp"

#include "../../filesystem/filesystem.h"
#include "../../eventManager.h"
#include "../../console.hpp"

#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>

#define NORMALIZE(v) if (std::abs(v.x) > 6.0f) v.x /= 1e2f; if (std::abs(v.y) > 6.0f) v.y /= 1e2f; if (std::abs(v.z) > 6.0f) v.z /= 1e2f;

#define NORM(n) n > 6.0f ? n / 1e2f : n

Filmbox::Filmbox(const std::string &path) : m_thread(std::make_unique<Thread>()) {
    m_thread->run(&Filmbox::doInBackground, this, path);
}

void Filmbox::setPostProcess(std::function<void(const FbxScene *)> func) {
    m_funcPost = func;
}

void Filmbox::doInBackground(const std::string &path) {
    if (!Filesystem::exists(path)) {
        return Console::Print(LogInfo::ERROR, "falied to opennable .fbx: " + path);
    }
    Assimp::Importer importer;

    const aiScene* scene = importer.ReadFile(path.c_str(), aiProcess_Triangulate | aiProcess_CalcTangentSpace | aiProcess_PopulateArmatureData | aiProcess_Triangulate);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        throw std::runtime_error("Erro ao carregar o modelo FBX: " + std::string(importer.GetErrorString()));
    }

    m_fbxScene = std::make_unique<FbxScene>(scene->mName.C_Str());

    sceneLoader(scene);

    importer.FreeScene();

    EventManager::addEvent(m_funcPost, m_fbxScene.get());
}

void Filmbox::sceneLoader(const aiScene *scene) {
    if (scene->HasMaterials()) {
        Console::Print(LogInfo::WARNING, "Loading material...");
        loadMaterial(scene);
    }

    if (scene->HasMeshes()) {
        Console::Print(LogInfo::WARNING, "Loading Mesh...");
        loadMesh(scene);
    }

    Console::Print(LogInfo::WARNING, "Loading Node...");

    auto mRootNode = scene->mRootNode;
    auto& fRootNode = m_fbxScene->m_rootNode;

    for (auto i=0;i<mRootNode->mNumChildren;++i) {
        processNode(mRootNode->mChildren[i], fRootNode);
    }
}

void Filmbox::loadMaterial(const aiScene *scene) {
    for (auto i=0;i<scene->mNumMaterials;++i) {
        auto vMaterial = scene->mMaterials[i];

        auto name = vMaterial->GetName();

        auto& material = m_fbxScene->addMaterial(name.C_Str());
        material.ao = 1.0f;
        
        aiColor3D diffuse;
        float value = 0.0f;

        if (vMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse) == AI_SUCCESS) {
            material.albedo = { diffuse.r, diffuse.g, diffuse.b, 1.0f };
        }

        if (vMaterial->Get(AI_MATKEY_REFLECTIVITY, value) == AI_SUCCESS) {
            material.metallic = value;
        }

        if (vMaterial->Get(AI_MATKEY_SHININESS, value) == AI_SUCCESS) {
            material.roughness = NORM(value);
        }

        vMaterial->Clear();
    }
}

void Filmbox::loadMesh(const aiScene *scene) {
    for (auto i=0;i<scene->mNumMeshes;++i) {
        processMesh(scene->mMeshes[i]);
    }
}

void Filmbox::processMesh(aiMesh *mMesh) {
    auto name = mMesh->mName;

    auto& mesh = m_fbxScene->addMesh(name.C_Str());

    auto bHasPosition = mMesh->HasPositions();
    auto bHasNormal = mMesh->HasNormals();
    auto bHasTexCoord = mMesh->HasTextureCoords(0);

    mesh.addMaterial(m_fbxScene->getMaterial(mMesh->mMaterialIndex));

    int mIdx = mesh.getMaterials().size() - 1;
    
    for (auto i=0;i<mMesh->mNumVertices;++i) {
        auto& vertice = mesh.addVertice(Vector3::zero, Vector3::zero, Vector2::zero, mIdx);

        if (bHasPosition) {
            auto mVertice = mMesh->mVertices[i];

            vertice.position = {mVertice.x, mVertice.y, mVertice.z};
        }

        if (bHasNormal) {
            auto mNormal = mMesh->mNormals[i];

            vertice.normal = {mNormal.x, mNormal.y, mNormal.z};
        }

        if (bHasTexCoord) {
            auto mTexCoord = mMesh->mTextureCoords[0][i];

            vertice.texCoord = {mTexCoord.x, mTexCoord.y};
        }
    }

    for (auto i=0;i<mMesh->mNumFaces;++i) {
        auto mFace = mMesh->mFaces[i];

        mesh.addIndice(mFace.mIndices[0]);
        mesh.addIndice(mFace.mIndices[1]);
        mesh.addIndice(mFace.mIndices[2]);
    }
}

void Filmbox::processNode(aiNode *mNode, FbxNode& node) {
    auto name = mNode->mName;

    auto& newNode = node.addNode(name.C_Str(), nullptr);

    aiVector3D scale;
    aiVector3D position;
    aiQuaternion rotation;
    mNode->mTransformation.Decompose(scale, rotation, position);

    NORMALIZE(scale)
    NORMALIZE(position)

    newNode.setPosition({position.x, position.y, position.z});
    newNode.setScale({scale.x, scale.y, scale.z});
    newNode.setRotation({rotation.w, rotation.x, rotation.y, rotation.z});

    for (auto i=0;i<mNode->mNumMeshes;++i) {
        auto index = mNode->mMeshes[i];

        newNode.addMesh(m_fbxScene->getMesh(index));
    }

    for (auto i=0;i<mNode->mNumChildren;++i) {
        auto child = mNode->mChildren[i];

        processNode(child, newNode);
    }
}
