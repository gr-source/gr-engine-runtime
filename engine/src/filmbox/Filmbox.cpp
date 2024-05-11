#include "Filmbox.h"

#include "../resources/Resources.h"
#include "../eventManager.h"

#include "../geometry/mesh.h"

#include <iostream>

#define NORMALIZE(vector) \
    if (std::abs(vector.x) > 6.0f) \
        vector.x /= 1e2f; \
    if (std::abs(vector.y) > 6.0f) \
        vector.y /= 1e2f; \
    if (std::abs(vector.z) > 6.0f) \
        vector.z /= 1e2f;

#define NORM(value) value > 6.0f ? value / 1e2f : value

#define DECOMPOSE(in, out) \
    aiVector3D s; \
    aiQuaternion r; \
    aiVector3D t; \
    in.Decompose(s, r, t); \
    out.scale = {s.x, s.y, s.z}; \
    out.rotation = {r.w, r.x, r.y, r.z}; \
    out.position = {t.x, t.y, t.z}; \



Filmbox::Filmbox(const std::string& path) : path(path), m_BoneCounter(0), m_changenewgameobject(nullptr), m_changeloading(nullptr), m_loaded(nullptr) {}

Filmbox::~Filmbox() {
    {
        std::lock_guard<std::mutex> lock(mt);
        importer.FreeScene();
    }

    if (m_thread.joinable()) {
        m_thread.join();
    }
}

void Filmbox::load() {
    m_thread = std::thread(&Filmbox::setupLoader, this);
}

void Filmbox::setupLoader() {
    const aiScene* scene = nullptr;

    {
        std::lock_guard<std::mutex> lock(mt);
        scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_CalcTangentSpace | aiProcess_GenBoundingBoxes);
    }

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        throw std::runtime_error("Erro ao carregar o modelo FBX: " + std::string(importer.GetErrorString()));
    }

    fbxScene.setname(scene->mName.C_Str());

    if (scene->HasCameras()) {
        loadCamera(scene);
    }

    if (scene->HasLights()) {
        loadLight(scene);
    }

    if (scene->HasMaterials()) {
        loadMaterials(scene);
    }

    if (scene->HasAnimations()) {
        // loadAnimations(scene);
    }

    if (scene->HasMeshes()) {
        loadMeshes(scene);
    }

    aiNode* rootNode = scene->mRootNode;

    loadNode(rootNode, scene, fbxScene.root);

    if (m_loaded != nullptr) {
        EventManager::addEvent(m_loaded, fbxScene);
    }
}

#include "../screen.h"

void Filmbox::loadCamera(const aiScene *scene) {
    for (u32 i=0;i<scene->mNumCameras;i++) {
        aiCamera* camera = scene->mCameras[i];

        std::string name = camera->mName.C_Str();

        fbxScene.m_cameras[name] = {
            Math::degree(camera->mHorizontalFOV),
            camera->mClipPlaneFar / 1e2f,
            camera->mClipPlaneNear / 1e2f,
            camera->mAspect,
            Vector3(camera->mUp.x, camera->mUp.y, camera->mUp.z),
            Vector3(camera->mLookAt.x, camera->mLookAt.y, camera->mLookAt.z)
        };
    }
}

FbxLight::LightType getLightType(aiLightSourceType type) {
    switch (type) {
    case aiLightSourceType::aiLightSource_DIRECTIONAL:
        return FbxLight::LightType::eDIRECTION;
    case aiLightSourceType::aiLightSource_POINT:
        return FbxLight::LightType::ePOINT;
    case aiLightSourceType::aiLightSource_SPOT:
        return FbxLight::LightType::eSPOT;
    default:
        throw std::runtime_error("Light type undefined");
    }
}

void Filmbox::loadLight(const aiScene *scene) {
    for (u32 i=0;i<scene->mNumLights;i++) {
        aiLight* light = scene->mLights[i];

        std::string name = light->mName.C_Str();

        fbxScene.m_lights[name] = {
            name,
            getLightType(light->mType),
            Color(light->mColorSpecular.r, light->mColorSpecular.g, light->mColorSpecular.b),
            light->mAttenuationConstant,
            light->mAttenuationLinear,
            light->mAttenuationQuadratic,
            Vector3(light->mDirection.x, light->mDirection.y, light->mDirection.z),
            Vector3(light->mUp.x, light->mUp.y, light->mUp.z),
            light->mAngleInnerCone,
            light->mAngleOuterCone
        };
    }
}

void Filmbox::loadAnimationKeys(Keyframe& keyFrame, const aiNodeAnim* nodeAnim) {
    loadAnimationKeyPositions(keyFrame, nodeAnim);
    loadAnimationKeyRotations(keyFrame, nodeAnim);
    loadAnimationKeyScales(keyFrame, nodeAnim);
}

void Filmbox::loadAnimationKeyPositions(Keyframe& keyFrame, const aiNodeAnim* nodeAnim) {
    for (std::uint32_t i = 0; i < nodeAnim->mNumPositionKeys; ++i) {
        aiVector3D aiPosition = nodeAnim->mPositionKeys[i].mValue;
        float timeStamp = nodeAnim->mPositionKeys[i].mTime;

        keyFrame.addKeyPosition(
            Vector3(aiPosition.x, aiPosition.y, aiPosition.z),
            timeStamp
        );
    }
}

void Filmbox::loadAnimationKeyRotations(Keyframe& keyFrame, const aiNodeAnim* nodeAnim) {
    for (std::uint32_t i = 0; i < nodeAnim->mNumRotationKeys; ++i) {
        aiQuaternion aiRotation = nodeAnim->mRotationKeys[i].mValue;
        float timeStamp = nodeAnim->mRotationKeys[i].mTime;

        keyFrame.addKeyRotation(
            Quaternion(aiRotation.w, aiRotation.x, aiRotation.y, aiRotation.z),
            timeStamp
        );
    }
}

void Filmbox::loadAnimationKeyScales(Keyframe& keyFrame, const aiNodeAnim* nodeAnim) {
    for (std::uint32_t i = 0; i < nodeAnim->mNumScalingKeys; ++i) {
        aiVector3D aiScale = nodeAnim->mScalingKeys[i].mValue;
        float timeStamp = nodeAnim->mScalingKeys[i].mTime;

        keyFrame.addKeyScale(
            Vector3(aiScale.x, aiScale.y, aiScale.z),
            timeStamp
        );
    }
}

void Filmbox::loadAnimations(const aiScene *scene) {
    for (u32 i=0;i<scene->mNumAnimations;i++) {
        aiAnimation* animation = scene->mAnimations[i];

        AnimationClip clip(animation->mName.C_Str(), animation->mDuration, animation->mTicksPerSecond);

        for (u32 j=0;j<animation->mNumChannels;j++) {
            aiNodeAnim* nodeAnim = animation->mChannels[j];

            Keyframe& keyframe = clip.addkeyframe(nodeAnim->mNodeName.C_Str());

            loadAnimationKeys(keyframe, nodeAnim);
        }

        fbxScene.m_animations.emplace(i, clip);
    }
}

void Filmbox::setupVertice(aiMesh *nodeMesh, Mesh &mesh) {
    const bool hasTangentBitangent = nodeMesh->HasTangentsAndBitangents();
    const bool hasTexCoord = nodeMesh->HasTextureCoords(0);
    const bool hasPositions = nodeMesh->HasPositions();
    const bool hasNormals = nodeMesh->HasNormals();

    for (u32 i=0;i<nodeMesh->mNumVertices;i++) {
        Vertex3D& vertex = mesh.addvertex();

        if (hasPositions) {
            aiVector3D position = nodeMesh->mVertices[i];

            vertex.position = {position.x, position.y, position.z};
        }

        if (hasNormals) {
            aiVector3D normal = nodeMesh->mNormals[i];

            vertex.normal = {normal.x, normal.y, normal.z};
        }

        if (hasTexCoord) {
            aiVector3D textCoord = nodeMesh->mTextureCoords[0][i];

            vertex.uv = {textCoord.x, textCoord.y};
        }

        if (hasTangentBitangent) {
            aiVector3D tangent = nodeMesh->mTangents[i];
            aiVector3D biTangent = nodeMesh->mBitangents[i];

            vertex.tangent = {tangent.x, tangent.y, tangent.z};
            vertex.bitangent = {biTangent.x, biTangent.y, biTangent.z};
        }

        vertex.materialIndex = nodeMesh->mMaterialIndex;
    }
}

void Filmbox::setupFace(aiMesh *nodeMesh, Mesh &mesh) {
    if (!nodeMesh->HasFaces()) return;

    for (u32 i=0;i<nodeMesh->mNumFaces;i++) {
        aiFace& face = nodeMesh->mFaces[i];

        for (u32 j=0;j<face.mNumIndices;j++) {
            mesh.addindice(face.mIndices[j]);
        }
    }
}

void Filmbox::setupSkeleton(aiNode* node, Bone* bone) {
    Bone* newBone = nullptr;
    if (node->mParent != nullptr && m_isBone[node->mName.C_Str()]) {
        Bone& boneMap = m_boneMap[node->mName.C_Str()];
        boneMap.m_parent = bone;

        newBone = &bone->addchild(boneMap);
    }

    for (u32 i=0;i<node->mNumChildren;i++) {
        aiNode* child = node->mChildren[i];

        if (newBone != nullptr) {
            setupSkeleton(child, newBone);
        } else {
            setupSkeleton(child, bone);
        }
    }
}

void Filmbox::setupBone(aiMesh *nodeMesh, Mesh &mesh, const aiScene* scene) {
    if (nodeMesh->HasBones()) {
        for (u32 i=0;i<nodeMesh->mNumBones;i++) {
            aiBone* nodeBone = nodeMesh->mBones[i];

            std::string boneName = nodeBone->mName.C_Str();

            Bone& bone = m_boneMap[boneName];
            bone.m_name = boneName;
            bone.id = i;

            DECOMPOSE(nodeBone->mOffsetMatrix, bone);

            for (u32 j=0;j<nodeBone->mNumWeights;j++) {
                aiVertexWeight vertexWeight = nodeBone->mWeights[j];

                Vertex3D& vertex = mesh.GetVertex(vertexWeight.mVertexId);

                for (u32 k=0;k<4;k++) {
                    if (vertex.boneIndex[k] == -1) {
                        vertex.boneIndex[k] = i;
                        vertex.weight[k] = vertexWeight.mWeight;
                        break;
                    }
                }
            }

            m_isBone[boneName] = true;
        }

        Skeleton& m_skeleton = mesh.m_skeleton;
        setupSkeleton(scene->mRootNode, &m_skeleton.m_root);
        
        m_boneMap.clear();
        m_isBone.clear();
        m_BoneCounter = 0;
    }
}

void Filmbox::loadvertices(aiMesh* pMesh, Mesh& mesh, const aiScene* scene) {
    setupVertice(pMesh, mesh);

    setupFace(pMesh, mesh);

    // setupBone(pMesh, mesh, scene);
}

void Filmbox::loadMeshes(const aiScene *scene) {
    for (u32 i=0;i<scene->mNumMeshes;i++) {
        aiMesh* pMesh = scene->mMeshes[i];

        aiAABB bounds = pMesh->mAABB;

        auto& mesh = fbxScene.addmesh(i, pMesh->mName.C_Str());

        mesh.m_boundBox = {Vector3(bounds.mMin.x, bounds.mMin.y, bounds.mMin.z), Vector3(bounds.mMax.x, bounds.mMax.y, bounds.mMax.z)};

        loadvertices(pMesh, mesh, scene);
    }
}

void Filmbox::loadMaterials(const aiScene *scene) {
    for (u32 i=0;i<scene->mNumMaterials;i++) {
        aiMaterial* pMaterial = scene->mMaterials[i];

        aiString name;
        if (pMaterial->Get(AI_MATKEY_NAME, name) != AI_SUCCESS) continue;

        auto& material = fbxScene.addmaterial(i, name.C_Str());

        loadMaterialTexture(pMaterial, material);

        aiColor3D diffuse;
        float value = 0.0f;

        if (AI_SUCCESS == pMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse)) {
            material.albedo = { diffuse.r, diffuse.g, diffuse.b, 1.0f };
        }

        if (AI_SUCCESS == pMaterial->Get(AI_MATKEY_REFLECTIVITY, value)) {
            material.metallic = value;
        }

        if (AI_SUCCESS == pMaterial->Get(AI_MATKEY_SHININESS, value)) {
            material.roughness = value;
        }

        // if (AI_SUCCESS == pMaterial->Get(AI_MATKEY_COLOR_AMBIENT, value)) {
            // material.ao = value;
        // }
    }
}

TexturePBR getTexturePBRType(aiTextureType type) {
    switch (type) {
    case aiTextureType_DIFFUSE:
        return TexturePBR::ALBEDO;
    case aiTextureType_NORMALS:
        return TexturePBR::NORMAL;
    case aiTextureType_SHININESS:
        return TexturePBR::ROUGHNESS;
    case aiTextureType_AMBIENT:
        return TexturePBR::AO;
    case aiTextureType_REFLECTION:
        return TexturePBR::METALLIC;
    default:
        std::cout << "texture do not loaded: " << type << std::endl;
        return TexturePBR::Non;
    }
}
/*
 * aiTextureType_DIFFUSE,
 * aiTextureType_SPECULAR,
 * aiTextureType_NORMALS,
 * aiTextureType_HEIGHT,
 * aiTextureType_AMBIENT,
 * aiTextureType_SHININESS,
 * aiTextureType_OPACITY,
 * aiTextureType_DISPLACEMENT,
 * aiTextureType_LIGHTMAP,
 * aiTextureType_REFLECTION,
 * aiTextureType_UNKNOWN
*/
void Filmbox::loadMaterialTexture(aiMaterial* pMaterial, Material& material) {
    aiString texturePath;
    aiTextureType types[] = {
        aiTextureType_DIFFUSE,
        aiTextureType_SPECULAR,
        aiTextureType_NORMALS,
        aiTextureType_HEIGHT,
        aiTextureType_AMBIENT,
        aiTextureType_SHININESS,
        aiTextureType_OPACITY,
        aiTextureType_DISPLACEMENT,
        aiTextureType_LIGHTMAP,
        aiTextureType_REFLECTION,
        aiTextureType_UNKNOWN
    };

    for (int i = 0; i < AI_TEXTURE_TYPE_MAX; i++) {
        aiTextureType type = types[i];
        if (pMaterial->GetTextureCount(type) > 0) {
            if (pMaterial->GetTexture(type, 0, &texturePath) != AI_SUCCESS) continue;

            TexturePBR pbrType = getTexturePBRType(type);
            if (pbrType == TexturePBR::Non) {
                continue;
            }// else if (pbrType == TexturePBR::ALBEDO) {
                material.texture[pbrType] = Resources::Load<Texture2D>(texturePath.C_Str());
            /*} else {
                material.texture[pbrType] = Resources::Load(texturePath.C_Str(), nullptr, Texture::eRGB, Texture::eRGB, Texture::eBYTE, Texture::eTRILINEAR, true);
            }*/
            // switch (pbrType) {
            // case TexturePBR::ALBEDO:
            //     continue;
            // case TexturePBR::Non:
            //     continue;
            // default:
            //     continue;
            // }
        }
    }
}

void Filmbox::setupNode(aiNode* node, const aiScene* scene, FbxNode& fbxNode) {
    DECOMPOSE(node->mTransformation, fbxNode)

    NORMALIZE(fbxNode.scale);
    NORMALIZE(fbxNode.position);

    for (u32 i=0;i<node->mNumMeshes;i++) {
        Mesh* mesh = fbxScene.findmesh(node->mMeshes[i]);
        if (!mesh) {
            Log::Write(LogType::eERROR, "[ Filmbox ] Mesh do not loaded");
            continue;
        }

        fbxNode.addmesh(*mesh);

        /* Improviso */
        aiMesh* m = scene->mMeshes[node->mMeshes[i]];

        fbxNode.materials[m->mMaterialIndex] = fbxScene.m_materials[m->mMaterialIndex];
    }
}

void Filmbox::loadNode(aiNode* node, const aiScene* scene, FbxNode& fbxNode) {
    FbxNode* newNode = nullptr;
    if (node->mParent != nullptr) {
        u8 type = FbxNode::eMODEL;

        auto light = fbxScene.m_lights.find(node->mName.C_Str());
        if (light != fbxScene.m_lights.end()) {
            type = FbxNode::eLIGHT;
        }
        auto camera = fbxScene.m_cameras.find(node->mName.C_Str());
        if (camera != fbxScene.m_cameras.end()) {
            type = FbxNode::eCAMERA;
        }

        newNode = &fbxNode.addchild(node->mName.C_Str(), &fbxNode, type);

        setupNode(node, scene, *newNode);

        if (m_changenewgameobject != nullptr)
            EventManager::addEvent(m_changenewgameobject, *newNode, &fbxNode, fbxScene);
    }

    for (u32 i=0;i<node->mNumChildren;i++) {
        aiNode* child = node->mChildren[i];

        if (newNode != nullptr) {
            loadNode(child, scene, *newNode);
        } else {
            loadNode(child, scene, fbxNode);
        }
    }
}

FbxScene& Filmbox::getscene() {
    return fbxScene;
}

void Filmbox::SetChangeGameObject(std::function<void(const FbxNode&, const FbxNode*, FbxScene&)> func) {
    m_changenewgameobject = func;
}

void Filmbox::SetLoadingStatus(std::function<void(u32, u32, const std::string&)> func) {
    m_changeloading = func;
}

void Filmbox::SetLoaded(const std::function<void(const FbxScene &)> func) {
    m_loaded = func;
}
