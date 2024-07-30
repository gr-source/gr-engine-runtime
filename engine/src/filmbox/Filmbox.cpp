#include "Filmbox.h"

#include "../resources/resources.h"
#include "../geometry/mesh.h"

#include "../memory.h"

#define NORMALIZE(vector) if (std::abs(vector.x) > 6.0f) vector.x /= 1e2f; if (std::abs(vector.y) > 6.0f) vector.y /= 1e2f; if (std::abs(vector.z) > 6.0f) vector.z /= 1e2f;

#define NORM(value) value > 6.0f ? value / 1e2f : value

#define DECOMPOSE(in, out) aiVector3D s; aiQuaternion r; aiVector3D t; in.Decompose(s, r, t); out.scale = {s.x, s.y, s.z}; out.rotation = {r.w, r.x, r.y, r.z}; out.position = {t.x, t.y, t.z}; 

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

void Filmbox::SetupVertice(aiMesh *aimesh, FbxMesh &mesh) {
    const bool hasTangentBitangent = aimesh->HasTangentsAndBitangents();
    const bool hasTexCoord = aimesh->HasTextureCoords(0);
    const bool hasPositions = aimesh->HasPositions();
    const bool hasNormals = aimesh->HasNormals();

    for (u32 i=0;i<aimesh->mNumVertices;i++) {
        FbxVertex3D& vertex = mesh.addVertex();

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

            mesh.addTriangle(face.mIndices[0], face.mIndices[1], face.mIndices[2]);
        }
    }
}

void Filmbox::LoadMaterials(const aiScene *scene) {
    /*
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
    */
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

        // material.texture[pbr] = Resources::LoadTexture<void(Filmbox::*)(Texture*texture), Filmbox*>(path.C_Str(), &Filmbox::TextureLoadSucessfull, this);
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


