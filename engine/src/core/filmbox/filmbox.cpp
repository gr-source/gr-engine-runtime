#include "filmbox.hpp"

#include "core/file/file.hpp"
#include "core/resources.h"

#include "core/event.hpp"

#include "console.hpp"

#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>

#define NORMALIZE(v) if (std::abs(v.x) > 6.0f) v.x /= 1e2f; if (std::abs(v.y) > 6.0f) v.y /= 1e2f; if (std::abs(v.z) > 6.0f) v.z /= 1e2f;

#define NORM(n) n > 6.0f ? n / 1e2f : n

Filmbox::Filmbox(const std::string &path) : m_path(path), m_thread(std::make_unique<Thread>()) {}

Filmbox::~Filmbox() {
    if (m_thread) {
        m_thread->stop();
    }
}

void Filmbox::setPostProcess(std::function<void(const FbxScene *, Filmbox *)> func) {
    m_funcPost = func;
}

void Filmbox::load() {
    m_thread->run(&Filmbox::doInBackground, this, m_path);
}

const bool Filmbox::isValid() const {
    return File::Exists(m_path);
}

void Filmbox::doInBackground(const std::string &path) {
    if (!File::Exists(path)) {
        return Console::log(LogInfo::ERROR, "falied to opennable .fbx: " + path);
    }
    Assimp::Importer importer;

    const aiScene* scene = importer.ReadFile(path.c_str(), aiProcess_Triangulate | aiProcess_CalcTangentSpace | aiProcess_PopulateArmatureData | aiProcess_Triangulate);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        throw std::runtime_error("Erro ao carregar o modelo FBX: " + std::string(importer.GetErrorString()));
    }

    m_fbxScene = std::make_unique<FbxScene>(scene->mName.C_Str());

    sceneLoader(scene);

    importer.FreeScene();

    if (m_funcPost) {
        Event::Add(m_funcPost, m_fbxScene.get(), this);
    }
}

void Filmbox::sceneLoader(const aiScene *scene) {
    if (scene->HasMaterials()) {
        Console::log(LogInfo::WARNING, "Loading material...");
        loadMaterial(scene);
    }

    if (scene->HasMeshes()) {
        Console::log(LogInfo::WARNING, "Loading Mesh...");
        loadMesh(scene);
    }

    if (scene->HasAnimations()) {
        Console::log(LogInfo::WARNING, "Loading Animations...");
        loadAnimation(scene);
    }

    Console::log(LogInfo::WARNING, "Loading Node...");

    auto mRootNode = scene->mRootNode;
    auto& fRootNode = m_fbxScene->m_rootNode;

    for (auto i=0;i<mRootNode->mNumChildren;++i) {
        processNode(mRootNode->mChildren[i], fRootNode, scene);
    }
}

void Filmbox::loadAnimation(const aiScene *scene) {
    for (auto i=0;i<scene->mNumAnimations;i++) {
        auto mAnimation = scene->mAnimations[i];

        auto name = mAnimation->mName;
        auto duration = mAnimation->mDuration;
        auto tick = mAnimation->mTicksPerSecond;

        auto &animation = m_fbxScene->addAnimation(name.C_Str(), duration, tick);

        for (auto j=0;j<mAnimation->mNumChannels;j++) {
            auto mChannel = mAnimation->mChannels[j];

            auto cName = mChannel->mNodeName;

            auto &channel = animation.addChannel(cName.C_Str());

            addKeys(mChannel, channel);
        }
    }
}

void Filmbox::addKeys(aiNodeAnim *mNodeAnim, FbxChannel &channel) {
    for (auto i=0;i<mNodeAnim->mNumPositionKeys;++i) {
        auto key = mNodeAnim->mPositionKeys[i];
        channel.addKeyPosition(static_cast<float>(key.mTime), Vector3(key.mValue.x, key.mValue.y, key.mValue.z));
    }

    for (auto i=0;i<mNodeAnim->mNumScalingKeys;++i) {
        auto key = mNodeAnim->mScalingKeys[i];
        channel.addKeyScale(static_cast<float>(key.mTime), Vector3(key.mValue.x, key.mValue.y, key.mValue.z));
    }

    for (auto i=0;i<mNodeAnim->mNumRotationKeys;++i) {
        auto key = mNodeAnim->mRotationKeys[i];
        channel.addKeyRotation(static_cast<float>(key.mTime), Quaternion(key.mValue.w, key.mValue.x, key.mValue.y, key.mValue.z));
    }
}

void Filmbox::loadMaterial(const aiScene *scene) {
    for (auto i=0;i<scene->mNumMaterials;++i) {
        auto vMaterial = scene->mMaterials[i];

        auto name = vMaterial->GetName();

        auto& material = m_fbxScene->addMaterial(name.C_Str());
        processTexture(vMaterial, material);

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
    }
}

void Filmbox::processTexture(aiMaterial *iMaterial, FbxMaterial &material) {
    auto count = iMaterial->GetTextureCount(aiTextureType_DIFFUSE);
    for (auto i=0;i<count;++i) {
        aiString path;
        if (iMaterial->GetTexture(aiTextureType_DIFFUSE, i, &path) != AI_SUCCESS) {
            continue;
        }

        material.addTexture(TextureType_Albedo, path.C_Str());
    }
}

void Filmbox::loadMesh(const aiScene *scene) {
    for (auto i=0;i<scene->mNumMeshes;++i) {
        processMesh(scene->mMeshes[i]);
    }
}

void Filmbox::processMesh(aiMesh *mMesh) {
    auto name = std::string(mMesh->mName.C_Str());

    auto mesh = m_fbxScene->findMesh(name);
    if (!mesh) {
        mesh = &m_fbxScene->addMesh(name, name);
    }

    inserMesh(mesh, mMesh);
}

void Filmbox::inserMesh(FbxMesh *mesh, aiMesh *mMesh) {
    auto bHasPosition = mMesh->HasPositions();
    auto bHasNormal = mMesh->HasNormals();
    auto bHasTexCoord = mMesh->HasTextureCoords(0);

    auto id = mesh->addMaterial(mMesh->mMaterialIndex);

    for (auto i=0;i<mMesh->mNumVertices;++i) {
        auto& vertice = mesh->addVertice(Vector3::zero, Vector3::zero, Vector2::zero, id);

        if (bHasPosition) {
            auto &mVertice = mMesh->mVertices[i];

            vertice.position = {mVertice.x, mVertice.y, mVertice.z};
        }

        if (bHasNormal) {
            auto &mNormal = mMesh->mNormals[i];

            vertice.normal = {mNormal.x, mNormal.y, mNormal.z};
        }

        if (bHasTexCoord) {
            auto &mTexCoord = mMesh->mTextureCoords[0][i];

            vertice.texCoord = {mTexCoord.x, mTexCoord.y};
        }
    }

    for (auto i=0;i<mMesh->mNumFaces;++i) {
        auto &mFace = mMesh->mFaces[i];

        if (mFace.mNumIndices > 3) {
            continue;
        }

        mesh->addTriangle(
            mFace.mIndices[0],
            mFace.mIndices[1],
            mFace.mIndices[2]
        );
    }

    if (mMesh->HasBones()) {
        for (auto i=0;i<mMesh->mNumBones;++i) {
            auto mBone = mMesh->mBones[i];

            auto name = mBone->mName;

            aiVector3D scale;
            aiQuaternion rotation;
            aiVector3D position;
            mBone->mOffsetMatrix.Decompose(scale, rotation, position);

            auto& fBone = mesh->addBone(name.C_Str(), i);
            fBone.setScale({scale.x, scale.y, scale.z});
            fBone.setPosition({position.x, position.y, position.z});
            fBone.setRotation({rotation.w, rotation.x, rotation.y, rotation.z});

            for (auto j=0;j<mBone->mNumWeights;++j) {
                auto weight = mBone->mWeights[j];

                auto& vertice = mesh->getVertice(weight.mVertexId);
                for (auto k=0;k<4;++k) {
                    if (vertice.boneIndex[k] != -1 || weight.mWeight <= 0.0f) {
                        continue;
                    }
                    vertice.weight[k] = static_cast<float>(weight.mWeight);
                    vertice.boneIndex[k] = i;
                    break;
                }
            }
        }
    }
}

void Filmbox::processNode(aiNode *mNode, FbxNode& node, const aiScene *scene) {
    auto name = mNode->mName;

    auto& newNode = node.addNode(name.C_Str(), nullptr);

    aiVector3D scale;
    aiVector3D position;
    aiQuaternion rotation;
    mNode->mTransformation.Decompose(scale, rotation, position);

    NORMALIZE(scale)
    // NORMALIZE(position)

    newNode.setPosition({position.x, position.y, position.z});
    newNode.setScale({scale.x, scale.y, scale.z});
    newNode.setRotation({rotation.w, rotation.x, rotation.y, rotation.z});

    for (auto i=0;i<mNode->mNumMeshes;++i) {
        auto name = scene->mMeshes[mNode->mMeshes[i]]->mName.C_Str();

        auto mesh = m_fbxScene->findMesh(name);
        if (!mesh) {
            std::cout << "Mesh do not found: " << name << std::endl;
            continue;
        }

        newNode.addMesh(*mesh);
    }

    for (auto i=0;i<mNode->mNumChildren;++i) {
        auto child = mNode->mChildren[i];

        processNode(child, newNode, scene);
    }
}





