#include "Filmbox.h"

#include "../resources/resources.h"
#include "../geometry/mesh.h"

#include "../eventManager.h"
#include "../memory.h"

#define NORMALIZE(vector) if (std::abs(vector.x) > 6.0f) vector.x /= 1e2f; if (std::abs(vector.y) > 6.0f) vector.y /= 1e2f; if (std::abs(vector.z) > 6.0f) vector.z /= 1e2f;

#define NORM(value) value > 6.0f ? value / 1e2f : value

#define DECOMPOSE(in, out) aiVector3D s; aiQuaternion r; aiVector3D t; in.Decompose(s, r, t); out.scale = {s.x, s.y, s.z}; out.rotation = {r.w, r.x, r.y, r.z}; out.position = {t.x, t.y, t.z}; 

/*
void Filmbox::load() {
    m_thread = std::thread(&Filmbox::setupLoader, this);
}

void Filmbox::setupLoader() {
    const aiScene* scene = nullptr;

    {
        std::lock_guard<std::mutex> lock(mt);
        scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_CalcTangentSpace | aiProcess_PopulateArmatureData | aiProcess_Triangulate);
    }

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        throw std::runtime_error("Erro ao carregar o modelo FBX: " + std::string(importer.GetErrorString()));
    }

    m_fbxScene->setname(scene->mName.C_Str());

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
        loadAnimations(scene);
    }

    if (scene->HasMeshes()) {
        loadMeshes(scene);
    }

    aiNode* rootNode = scene->mRootNode;

    loadNode(rootNode, scene, m_fbxScene->root);

    {
        std::lock_guard<std::mutex> lock(mt);
        if (m_loaded != nullptr) {
            EventManager::addEvent(m_loaded, fbxScene);
        }
    }
}

void Filmbox::loadCamera(const aiScene *scene) {
    for (u32 i=0;i<scene->mNumCameras;i++) {
        aiCamera* camera = scene->mCameras[i];

        std::string name = camera->mName.C_Str();

        m_fbxScene->m_cameras[name] = {
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

        m_fbxScene->m_lights[name] = {
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

void Filmbox::loadAnimations(const aiScene *scene) {
    for (u32 i=0;i<scene->mNumAnimations;i++) {
        aiAnimation* animation = scene->mAnimations[i];

        std::string name = animation->mName.C_Str();

        auto& clip = m_fbxScene->addanimation(i, name);
        clip.setduration(animation->mDuration);
        clip.setmaxtick(animation->mTicksPerSecond);

        for (u32 j=0;j<animation->mNumChannels;j++) {
            aiNodeAnim* nodeAnim = animation->mChannels[j];

            Keyframe& keyframe = clip.addkeyframe(nodeAnim->mNodeName.C_Str());

            loadAnimationKeys(keyframe, nodeAnim);
        }
    }
}

void Filmbox::loadAnimationKeys(Keyframe& keyFrame, const aiNodeAnim* nodeAnim) {
    loadAnimationKeyPositions(keyFrame, nodeAnim);
    loadAnimationKeyRotations(keyFrame, nodeAnim);
    loadAnimationKeyScales(keyFrame, nodeAnim);
}

void Filmbox::loadAnimationKeyPositions(Keyframe& keyFrame, const aiNodeAnim* nodeAnim) {
    for (u32 i = 0; i < nodeAnim->mNumPositionKeys; ++i) {
        aiVector3D aiPosition = nodeAnim->mPositionKeys[i].mValue;
        float timeStamp = nodeAnim->mPositionKeys[i].mTime;

        keyFrame.addKeyPosition(
            Vector3(aiPosition.x, aiPosition.y, aiPosition.z),
            timeStamp
        );
    }
}

void Filmbox::loadAnimationKeyRotations(Keyframe& keyFrame, const aiNodeAnim* nodeAnim) {
    for (u32 i = 0; i < nodeAnim->mNumRotationKeys; ++i) {
        aiQuaternion aiRotation = nodeAnim->mRotationKeys[i].mValue;
        float timeStamp = nodeAnim->mRotationKeys[i].mTime;

        keyFrame.addKeyRotation(
            Quaternion(aiRotation.w, aiRotation.x, aiRotation.y, aiRotation.z),
            timeStamp
        );
    }
}

void Filmbox::loadAnimationKeyScales(Keyframe& keyFrame, const aiNodeAnim* nodeAnim) {
    for (u32 i = 0; i < nodeAnim->mNumScalingKeys; ++i) {
        aiVector3D aiScale = nodeAnim->mScalingKeys[i].mValue;
        float timeStamp = nodeAnim->mScalingKeys[i].mTime;

        keyFrame.addKeyScale(
            Vector3(aiScale.x, aiScale.y, aiScale.z),
            timeStamp
        );
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

        if (face.mNumIndices > 3) {
            Log::error() << "Face não é um triangulo" << Log::end;
        }

        for (u32 j=0;j<face.mNumIndices;j++) {
            mesh.addindice(face.mIndices[j]);
        }
    }
}

void Filmbox::setupSkeleton(aiNode* node, Bone*& bone) {
    Bone* newBone = nullptr;
    if (node->mParent != nullptr) {
        if (bone == nullptr) {
            newBone = bone = new Bone(node->mName.C_Str());
        } else {
            newBone = &bone->addchild(node->mName.C_Str());
        }
    }
    
    for (u32 i=0;i<node->mNumChildren;i++) {
        aiNode* child = node->mChildren[i];

        setupSkeleton(child, newBone);
    }
}

void Filmbox::setupBone(FbxNode& fbxNode, Mesh &mesh, aiMesh *nodeMesh, const aiScene* scene) {
    if (!nodeMesh->HasBones()) {
        return;
    }

    std::vector<std::string> boneNames;

    for (u32 i=0;i<nodeMesh->mNumBones;i++) {
        aiBone* pBone = nodeMesh->mBones[i];

        std::string name = pBone->mName.C_Str();

        boneNames.emplace_back(name);

        aiNode* armature = pBone->mArmature;

        Skeleton* skeleton = fbxScene.findSkeleton(armature->mName.C_Str());
        if (!skeleton) {
            skeleton = &fbxScene.addskeleton(armature->mName.C_Str());
            
            setupSkeleton(armature, skeleton->m_root);

            Log::info() << "New Skeleton: " << skeleton->getname() << Log::end;
        }

        Bone* bone = skeleton->findBone(name);
        if (!bone) {
            Log::error() << "Bone not found in skeleton" << std::endl;
            continue;
        }
        bone->m_name = name;
        bone->id = i;

        DECOMPOSE(pBone->mOffsetMatrix, (*bone));

        for (u32 j=0;j<pBone->mNumWeights;j++) {
            aiVertexWeight& vertexWeight = pBone->mWeights[j];

            Vertex3D& vertex = mesh.GetVertex(vertexWeight.mVertexId);

            addBoneData(vertex, bone->id, vertexWeight.mWeight);
        }
    }

    for (auto& skeleton : fbxScene.m_skeletons) {
        bool allBonesFound = true;

        for (const auto& name : boneNames) {
            auto* bone = skeleton.findBone(name);
            if (!bone) {
                allBonesFound = false;
                break;
            }
        }

        if (allBonesFound) {
            fbxNode.m_skeleton = new Skeleton(skeleton);
            break;
        }
    }

    auto& animations = fbxScene.getanimations();

    for (auto& entry : animations) {
        auto& anim = entry.second;

        bool allBonesFound = true;

        for (auto& keyframe : anim.getKeyList()) {
            auto* bone = fbxNode.m_skeleton->findBone(keyframe.getname());
            if (!bone) {
                allBonesFound = false;
                break;
            }
        }

        if (allBonesFound) {
            fbxNode.m_animations.emplace_back(anim);
            break;
        }
    }
    

    m_BoneCounter = 0;
}

void Filmbox::addBoneData(Vertex3D &vertex, u32 index, float weight) {
    if (weight == 0.0f || index >= 100) {
        Log::error() << "Bone invalid wight or index. " << weight << " " << index << Log::end;
        return;
    }

    for (u32 k=0;k<4;k++) {
        if (vertex.boneIndex[k] == -1 && vertex.boneIndex[k] != index) {
            vertex.boneIndex[k] = index;
            vertex.weight[k] = weight;
            break;
        }
    }
}

void Filmbox::loadvertices(FbxNode& fbxNode, Mesh& mesh, aiMesh* pMesh, const aiScene* scene) {
    setupVertice(pMesh, mesh);

    setupFace(pMesh, mesh);

    setupBone(fbxNode, mesh, pMesh, scene);
}

void Filmbox::loadMeshes(const aiScene *scene) {
    for (u32 i=0;i<scene->mNumMeshes;i++) {
        aiMesh* pMesh = scene->mMeshes[i];

        auto& mesh = fbxScene.addmesh(i, pMesh->mName.C_Str());

        loadvertices(pMesh, mesh, scene);
    }
}

void Filmbox::loadMaterials(const aiScene *scene) {
    for (u32 i=0;i<scene->mNumMaterials;i++) {
        aiMaterial* pMaterial = scene->mMaterials[i];

        aiString name;
        if (pMaterial->Get(AI_MATKEY_NAME, name) != AI_SUCCESS) continue;

        auto& material = fbxScene.addmaterial(i, name.C_Str());
// 
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
            material.roughness = NORM(value);
        }

        // if (AI_SUCCESS == pMaterial->Get(AI_MATKEY_COLOR_AMBIENT, value)) {
            // material.ao = value;
        // }
    }
}

const char* get_enum_name(aiTextureType type) {
    #define GET_ENUM_NAME(e) case e:    return #e
    switch (type) {
        GET_ENUM_NAME(aiTextureType_NONE);
        GET_ENUM_NAME(aiTextureType_DIFFUSE);
        GET_ENUM_NAME(aiTextureType_SPECULAR);
        GET_ENUM_NAME(aiTextureType_AMBIENT);
        GET_ENUM_NAME(aiTextureType_EMISSIVE);
        GET_ENUM_NAME(aiTextureType_HEIGHT);
        GET_ENUM_NAME(aiTextureType_NORMALS);
        GET_ENUM_NAME(aiTextureType_SHININESS);
        GET_ENUM_NAME(aiTextureType_OPACITY);
        GET_ENUM_NAME(aiTextureType_DISPLACEMENT);
        GET_ENUM_NAME(aiTextureType_LIGHTMAP);
        GET_ENUM_NAME(aiTextureType_REFLECTION);
        GET_ENUM_NAME(aiTextureType_BASE_COLOR);
        GET_ENUM_NAME(aiTextureType_NORMAL_CAMERA);
        GET_ENUM_NAME(aiTextureType_EMISSION_COLOR);
        GET_ENUM_NAME(aiTextureType_METALNESS);
        GET_ENUM_NAME(aiTextureType_DIFFUSE_ROUGHNESS);
        GET_ENUM_NAME(aiTextureType_AMBIENT_OCCLUSION);
        GET_ENUM_NAME(aiTextureType_SHEEN);
        GET_ENUM_NAME(aiTextureType_CLEARCOAT);
        GET_ENUM_NAME(aiTextureType_TRANSMISSION);
        GET_ENUM_NAME(aiTextureType_UNKNOWN);
    default:
        return "Invalid";
    }
    #undef GET_ENUM_NAME
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
        Log::error() << "texture do not loaded: " << get_enum_name(type) << Log::end;
        return TexturePBR::Non;
    }
}

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
            TextureArgs args;
            if (pbrType == TexturePBR::Non) {
                continue;
            }
            material.texture[pbrType] = Resources::LoadTexture<void(Filmbox::*)(Texture*texture), Filmbox*>(texturePath.C_Str(), &Filmbox::TextureLoadSucessfull, this);
        }
    }
}

void Filmbox::setupNode(aiNode* node, const aiScene* scene, FbxNode& fbxNode) {
    DECOMPOSE(node->mTransformation, fbxNode)

    NORMALIZE(fbxNode.scale);
    NORMALIZE(fbxNode.position);

    for (u32 i=0;i<node->mNumMeshes;i++) {
        aiMesh* pMesh = scene->mMeshes[node->mMeshes[i]];

        auto& mesh = fbxNode.addmesh(pMesh->mName.C_Str());

        loadvertices(fbxNode, mesh, pMesh, scene);

        fbxNode.materials[pMesh->mMaterialIndex] = fbxScene.m_materials[pMesh->mMaterialIndex];
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

        Skeleton* it = fbxScene.findSkeleton(child->mName.C_Str());
        if (it != nullptr) {
            continue;
        }

        if (newNode != nullptr) {
            loadNode(child, scene, *newNode);
        } else {
            loadNode(child, scene, fbxNode);
        }
    }
}

void Filmbox::TextureLoadSucessfull(Texture *texture) {
    // Log::info << "Texture loadde sucess: " << texture->getpath() << Log::end;
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
*/

Filmbox *Filmbox::Load(const std::string &path) {
    Filmbox* filmbox = new Filmbox();
    filmbox->m_path = path;
    filmbox->process();

    return filmbox;
}

Filmbox::Filmbox() : m_path("/"), m_fbxScene(nullptr) {}

const FbxScene *Filmbox::getScene() {
    return m_fbxScene.get();
}

void Filmbox::destroy() {
    delete this;
}

void Filmbox::process() {
    Assimp::Importer importer;

    const aiScene* scene = importer.ReadFile(m_path, aiProcess_Triangulate | aiProcess_CalcTangentSpace | aiProcess_PopulateArmatureData | aiProcess_Triangulate);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        throw std::runtime_error("Erro ao carregar o modelo FBX: " + std::string(importer.GetErrorString()));
    }

    m_fbxScene = std::make_unique<FbxScene>(scene->mName.C_Str());


    StartupScene(scene);

    importer.FreeScene();
}

void Filmbox::StartupScene(const aiScene *scene) {
    if (scene->HasAnimations()) {
        Log::info() << "Process: Animations" << Log::end;

        LoadAnimations(scene);
    }

    if (scene->HasCameras()) {
        Log::info() << "Cameras loaded" << Log::end;
    }

    if (scene->HasLights()) {
        Log::info() << "Lights loaded" << Log::end;
    }

    if (scene->HasTextures()) {
        Log::info() << "Textures loaded" << Log::end;

        LoadTextures(scene);
    }
    
    if (scene->HasMaterials()) {
        Log::info() << "Materials loaded" << Log::end;

        LoadMaterials(scene);
    }

    if (scene->HasMeshes()) {
        Log::info() << "Meshs loaded" << Log::end;

        LoadMeshes(scene);
    }
    
    ProcessNodes(scene);
}

void Filmbox::ProcessNodes(const aiScene *scene) {
    auto* root = scene->mRootNode;

    FbxNode* fbxRoot = m_fbxScene->CreateRoot(root->mName.C_Str());

    for (u32 i=0;i<root->mNumChildren;i++) {
        auto child = root->mChildren[i];

        ProcessChild(child, fbxRoot);
    }
}

void Filmbox::ProcessChild(aiNode *ainode, FbxNode* fbxNode) {
    auto armature = m_fbxScene->findSkeleton(ainode->mName.C_Str());
    if (armature) {
        return;
    }

    std::string name = ainode->mName.C_Str();

    FbxNode* newNode = &fbxNode->addchild(name);

    SetupNode(ainode, newNode);

    for (u32 i=0;i<ainode->mNumChildren;i++) {
        aiNode* child = ainode->mChildren[i];

        ProcessChild(child, newNode);
    }
}

void Filmbox::SetupNode(aiNode *ainode, FbxNode *fbxNode) {
    DECOMPOSE(ainode->mTransformation, (*fbxNode));

    NORMALIZE(fbxNode->position);
    NORMALIZE(fbxNode->rotation);
    NORMALIZE(fbxNode->scale);

    for (u32 i=0;i<ainode->mNumMeshes;i++) {
        auto index = ainode->mMeshes[i];

        auto mesh = m_fbxScene->findmesh(index);
        if (!mesh) {
            Log::error() << "Mesh not found: " << index << Log::end;
            continue;
        }

        fbxNode->addmesh(*mesh);

        for (const auto& id : mesh->getMaterials()) {
            auto material = m_fbxScene->findMaterial(id);
            if (!material) {
                Log::error() << "Material not found: " << id << Log::end;
                continue;
            }

            auto m = fbxNode->findMaterial(id);
            if (m) {
                continue;
            }

            fbxNode->addMaterial(id, *material);
        }
    }
}

void Filmbox::LoadMeshes(const aiScene *scene) {
    for (u32 i=0;i<scene->mNumMeshes;i++) {
        aiMesh* aimesh = scene->mMeshes[i];

        std::string name = aimesh->mName.C_Str();

        auto& mesh = m_fbxScene->addmesh(i, name);

        SetupVertice(aimesh, mesh);

        SetupBone(aimesh);
    }
}

void Filmbox::SetupBone(aiMesh *aimesh) {
    if (!aimesh->HasBones()) return;

    Log::info() << "Mesh contais bone. prepare to loadding bones!" << Log::end;

    for (u32 i=0;i<aimesh->mNumBones;i++) {
        aiBone* aibone = aimesh->mBones[i];

        std::string name = aibone->mName.C_Str();

        aiNode* ainode = aibone->mArmature;

        auto armature = m_fbxScene->findSkeleton(ainode->mName.C_Str());
        if (armature == nullptr) {
            armature = &m_fbxScene->addskeleton(ainode->mName.C_Str());

            SetupSkeleton(aibone->mArmature, armature->m_root);
        }

    }
}

void Filmbox::SetupSkeleton(aiNode* node, Bone*& bone) {
    std::cout << "Load new armature: " << node->mName.C_Str() << std::endl;

    // Bone* newBone = nullptr;
    // if (node->mParent != nullptr) {
    //     if (bone == nullptr) {
    //         newBone = bone = new Bone(node->mName.C_Str());
    //     } else {
    //         newBone = &bone->addchild(node->mName.C_Str());
    //     }
    // }
    
    // for (u32 i=0;i<node->mNumChildren;i++) {
    //     aiNode* child = node->mChildren[i];

    //     setupSkeleton(child, newBone);
    // }
}

void Filmbox::SetupVertice(aiMesh *aimesh, Mesh &mesh) {
    const bool hasTangentBitangent = aimesh->HasTangentsAndBitangents();
    const bool hasTexCoord = aimesh->HasTextureCoords(0);
    const bool hasPositions = aimesh->HasPositions();
    const bool hasNormals = aimesh->HasNormals();

    for (u32 i=0;i<aimesh->mNumVertices;i++) {
        Vertex3D& vertex = mesh.addvertex();

        if (hasPositions) {
            aiVector3D position = aimesh->mVertices[i];

            vertex.position = {position.x, position.y, position.z};
        }

        if (hasNormals) {
            aiVector3D normal = aimesh->mNormals[i];

            vertex.normal = {normal.x, normal.y, normal.z};
        }

        if (hasTexCoord) {
            aiVector3D textCoord = aimesh->mTextureCoords[0][i];

            vertex.uv = {textCoord.x, textCoord.y};
        }

        if (hasTangentBitangent) {
            aiVector3D tangent = aimesh->mTangents[i];
            aiVector3D biTangent = aimesh->mBitangents[i];

            vertex.tangent = {tangent.x, tangent.y, tangent.z};
            vertex.bitangent = {biTangent.x, biTangent.y, biTangent.z};
        }

        auto material = mesh.findMaterial(aimesh->mMaterialIndex);
        if (!material) {
            mesh.addMaterial(aimesh->mMaterialIndex);
        }
        vertex.materialIndex = aimesh->mMaterialIndex;
    }

    if (aimesh->HasFaces()) {
        for (u32 i=0;i<aimesh->mNumFaces;i++) {
            aiFace& face = aimesh->mFaces[i];

            if (face.mNumIndices > 3) {
                Log::error() << "Mesh do not is triangulate!" << Log::end;
                continue;
            }

            mesh.addtriangle(face.mIndices[0], face.mIndices[1], face.mIndices[2]);
        }
    }
}

void Filmbox::LoadMaterials(const aiScene *scene) {
    for (u32 i=0;i<scene->mNumMaterials;i++) {
        aiMaterial* aimaterial = scene->mMaterials[i];

        aiString name;
        if (aimaterial->Get(AI_MATKEY_NAME, name) != AI_SUCCESS) continue;

        auto& material = m_fbxScene->addmaterial(i, name.C_Str());

        SetupTexture(aimaterial, material);

        aiColor3D diffuse;
        float value = 0.0f;

        if (aimaterial->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse) == AI_SUCCESS) {
            material.albedo = { diffuse.r, diffuse.g, diffuse.b, 1.0f };
        }

        if (aimaterial->Get(AI_MATKEY_REFLECTIVITY, value) == AI_SUCCESS) {
            material.metallic = value;
        }

        if (aimaterial->Get(AI_MATKEY_SHININESS, value) == AI_SUCCESS) {
            material.roughness = NORM(value);
        }

        aimaterial->Clear();
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
        Log::error() << "texture do not loaded: " << get_enum_name(type) << Log::end;
        return TexturePBR::Non;
    }
}

void Filmbox::SetupTexture(aiMaterial *aimaterial, Material &material) {
    std::vector<aiTextureType> m_texturesTypes{aiTextureType_NONE, aiTextureType_DIFFUSE, aiTextureType_SPECULAR, aiTextureType_AMBIENT, aiTextureType_EMISSIVE, aiTextureType_HEIGHT, aiTextureType_NORMALS, aiTextureType_SHININESS, aiTextureType_OPACITY, aiTextureType_DISPLACEMENT, aiTextureType_LIGHTMAP, aiTextureType_REFLECTION, aiTextureType_BASE_COLOR, aiTextureType_NORMAL_CAMERA, aiTextureType_EMISSION_COLOR, aiTextureType_METALNESS, aiTextureType_DIFFUSE_ROUGHNESS, aiTextureType_AMBIENT_OCCLUSION, aiTextureType_SHEEN, aiTextureType_CLEARCOAT, aiTextureType_TRANSMISSION, aiTextureType_UNKNOWN};

    for (const auto& type : m_texturesTypes) {
        u32 count = aimaterial->GetTextureCount(type);
        if (count <= 0) continue;

        aiString path;
        if (aimaterial->GetTexture(type, 0, &path) != AI_SUCCESS) continue;

        auto pbr = getTexturePBRType(type);
        if (pbr == TexturePBR::Non) continue;

        material.texture[pbr] = Resources::LoadTexture<void(Filmbox::*)(Texture*texture), Filmbox*>(path.C_Str(), &Filmbox::TextureLoadSucessfull, this);
    }
}

void Filmbox::TextureLoadSucessfull(Texture *texture) {}

void Filmbox::LoadTextures(const aiScene *scene) {
    /*

    for (u32 i = 0; i < scene->mNumTextures; i++) {
        aiTexture* texture = scene->mTextures[i];
        if (!texture || !texture->pcData) {
            std::cerr << "Texture or texture data is null!" << std::endl;
            continue;
        }

        // If mHeight is 0, the texture is compressed.
        if (texture->mHeight == 0) {
            std::cout << "Compressed texture data: ";
            for (u32 j = 0; j < texture->mWidth; j++) {
                std::cout << static_cast<int>(texture->pcData[j].r) << " ";
            }
            std::cout << std::endl;
        } else {
            // The texture is uncompressed.
            std::cout << "Uncompressed texture data: ";
            for (u32 y = 0; y < texture->mHeight; y++) {
                for (u32 x = 0; x < texture->mWidth; x++) {
                    aiTexel& pixel = texture->pcData[y * texture->mWidth + x];
                    std::cout << "A: " << static_cast<int>(pixel.a) << " "
                              << "R: " << static_cast<int>(pixel.r) << " "
                              << "G: " << static_cast<int>(pixel.g) << " "
                              << "B: " << static_cast<int>(pixel.b) << " | ";
                }
            }
            std::cout << std::endl;
        }
    }*/
}

void Filmbox::LoadAnimations(const aiScene *scene) {
    for (u32 i=0;i<scene->mNumAnimations;i++) {
        aiAnimation* animation = scene->mAnimations[i];

        std::string name = animation->mName.C_Str();

        auto& anim = m_fbxScene->addanimation(name);
        anim.setduration(static_cast<float>(animation->mDuration));
        anim.setticks(static_cast<float>(animation->mTicksPerSecond));

        for (u32 j=0;j<animation->mNumChannels;j++) {
            aiNodeAnim* nodeAnim = animation->mChannels[j];

            std::string nodeName = nodeAnim->mNodeName.C_Str();

            auto& keyframe = anim.addkeyframe(nodeName);

            LoadKeyframeScale(nodeAnim, keyframe);

            LoadKeyframeRotation(nodeAnim, keyframe);

            LoadKeyframePosition(nodeAnim, keyframe);
        }

        Log::info() << "Animation: " << name << Log::end;
        Log::warning() << "Total keyframes: " << anim.getKeyframecout() << Log::end;
    }
}

void Filmbox::LoadKeyframeScale(aiNodeAnim *nodeAnim, Keyframe &key) {
    for (int i=0;i<nodeAnim->mNumScalingKeys;i++) {
        aiVectorKey& vecKey = nodeAnim->mScalingKeys[i];

        key.addKeyScale({vecKey.mValue.x, vecKey.mValue.y, vecKey.mValue.z}, static_cast<float>(vecKey.mTime));
    }
}

void Filmbox::LoadKeyframeRotation(aiNodeAnim *nodeAnim, Keyframe &key) {
    for (int i=0;i<nodeAnim->mNumRotationKeys;i++) {
        aiQuatKey& quatKey = nodeAnim->mRotationKeys[i];

        key.addKeyRotation({quatKey.mValue.w, quatKey.mValue.x, quatKey.mValue.y, quatKey.mValue.z}, static_cast<float>(quatKey.mTime));
    }
}

void Filmbox::LoadKeyframePosition(aiNodeAnim *nodeAnim, Keyframe &key) {
    for (int i=0;i<nodeAnim->mNumPositionKeys;i++) {
        aiVectorKey& vecKey = nodeAnim->mPositionKeys[i];

        key.addKeyPosition({vecKey.mValue.x, vecKey.mValue.y, vecKey.mValue.z}, static_cast<float>(vecKey.mTime));
    }
}


