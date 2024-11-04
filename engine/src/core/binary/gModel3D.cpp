#include "gModel3D.hpp"

#include "gHeader.hpp"

#include "core/threadpool.hpp"
#include "core/console.hpp"
#include "core/scene.hpp"

#include "core/file/stream.hpp"
#include "core/file/file.hpp"

#include "core/filmbox/filmbox.hpp"

#include "ecs/gameObject.h"

#include "script/transform.hpp"

std::list<std::tuple<std::string, std::string, Component *>> gModel3D::m_componentTemp;

std::unordered_map<std::string, GameObject *> gModel3D::m_gameObjectTemp;

void gModel3D::ImportFBX(const std::string &path) {
    Threadpool::AddTask(&Filmbox::Load, path, &gModel3D::SetupScene);
}

void gModel3D::ExportGRMX(const std::string &path, const GameObject *gameObject) {
    auto name = gameObject->getName();

    LOG_INFO_ARGS("[ gModel3D ] Writing GameObject: %s", name.c_str());

    Stream stream(path + "/" + name + ".gmdx", std::ios_base::out | std::ios_base::binary);

    // Header file
    {
        gHeader header{{'g', 'r', 'm', 'x'}, 1.0f, 0};
        stream.writeS(&header, 1, false);
    }

    // data
    WriteData(&stream, gameObject);

    LOG_INFO_ARGS("[ gModel3D ] Write GameObject(%s) sucessfull.", name.c_str());
}

void gModel3D::ImportGRMX(const std::string &path) {
    Stream stream(path, std::ios_base::in | std::ios_base::binary);
    if (!stream.is_open()) {
        return LOG_ERROR_ARGS("[ gModel3D ] Falied to open file: %s", path.c_str());
    }

    gHeader header;
    if (!stream.readS(&header, 1)) {
        return LOG_ERROR("[ gModel3D ] Falied to read header binary.");
    }
    
    if (header.signature[0] != 'g' && header.signature[1] != 'r' && header.signature[2] != 'm' && header.signature[3] != 'x') {
        return LOG_ERROR("[ gModel3D ] Invalid binary type.");
    }

    LOG_INFO("[ gModel3D ] GRMX loading model.");

    ReadData(&stream);

    LOG_INFO("[ gModel3D ] Scene loaded sucess full.");
}

int gModel3D::ReadData(Stream *stream) {
    GameObject *gameObject = nullptr;

    while (true) {
        gStructData segment;
        if (!stream->readS(&segment, 1)) {
            break;
        }

        if (segment.type == gHeaderType_None && segment.sizeData == 0) {
            break;
        }

        auto next_position = stream->tellg() + static_cast<std::streampos>(segment.sizeData);

        auto result = -1;
        switch (segment.type) {
        case gHeaderType_Object:
            result = ReadGameObject(stream, nullptr, &gameObject);
            break;
        case gHeaderType_Script:
            result = ReadScript(stream);
            break;
        default:
            break;
        }

        if (stream->tellg() != next_position)
            stream->seekg(next_position, std::ios_base::beg);
    }

    if (gameObject) {
        Scene::GetCurrentScene()->insert(gameObject);

        gameObject->initialize();
    }

    m_gameObjectTemp.clear();
    
    return 0;
}

int gModel3D::ReadGameObject(Stream *stream, Transform *parent, GameObject **out) {
    std::size_t nsize = 0;

    // write-name
    if (!stream->readS(&nsize, 1)) {
        return 1;
    }

    std::string name(nsize, '\0');
    if (!stream->readS(name.data(), nsize)) {
        return 2;
    }

    // write-tag
    if (!stream->readS(&nsize, 1)) {
        return 3;
    }

    std::string tag(nsize, '\0');
    if (!stream->readS(tag.data(), nsize)) {
        return 4;
    }

    auto bActive = false;
    auto bEnable = false;

    // write-status
    {
        if (!stream->readS(&bActive, 1)) {
            return 5;
        }

        if (!stream->readS(&bEnable, 1)) {
            return 6;
        }
    }

    auto gameObject = GameObject::Create(name, tag);
    if (!(*out)) {
        *out = gameObject;
    }
    m_gameObjectTemp.emplace(name, gameObject);

    gameObject->setActive(bActive);
    gameObject->setEnable(bEnable);

    auto transform = gameObject->getTransform();
    if (parent) {
        transform->setParent(parent);
    }

    std::size_t children_size = 0;
    if (!stream->readS(&children_size, 1)) {
        return 7;
    }
    
    int result = 0;
    for (int i=0;i<children_size;i++) {
        result = ReadGameObject(stream, transform, out);
        if (result != 0) {
            break;
        }
    }
    return result;
}

int gModel3D::ReadScript(Stream *stream) {
    std::size_t size = 0;

    if (!stream->readS(&size, 1)) {
        return 1;
    }

    for (std::size_t i=0;i<size;i++) {
        std::size_t nsize = 0;

        // GameObject - name
        if (!stream->readS(&nsize, 1)) {
            return 2;
        }
        
        std::string objectName(nsize, '\0');
        if (!stream->readS(objectName.data(), nsize)) {
            return 3;
        }

        // GameObject - ScriptName
        if (!stream->readS(&nsize, 1)) {
            return 4;
        }
        
        std::string componentName(nsize, '\0');
        if (!stream->readS(componentName.data(), nsize)) {
            return 5;
        }

        std::size_t stream_size = 0;
        if (!stream->readS(&stream_size, 1)) {
            return 6;
        }
        auto nextPosition = stream->tellg() + static_cast<std::streampos>(stream_size);

        // deserialize
        auto it = m_gameObjectTemp.find(objectName);
        if (it != m_gameObjectTemp.end()) {
            auto gameObject = it->second;

            auto entt = gameObject->getEntity();

            auto component = entt->hasComponent(componentName) ? entt->getComponent(componentName) : entt->addComponent(componentName);
            if (component) {
                component->Deserialize(stream);
            }
        }

        if (stream->tellg() != nextPosition)
            stream->seekg(nextPosition, std::ios_base::beg);
    }
    return 0;
}

int gModel3D::WriteData(Stream *stream, const GameObject *gameObject) {
    int result = 0;
    gStructData segment = {gHeaderType_None, 0};

    // GameObject - Node
    {
        result = WriteGameObject(stream, gameObject);
        if (result != 0) {
            return 1;
        }

        segment = {gHeaderType_Object, static_cast<std::size_t>(stream->getDataSize())};
        if (!stream->writeS(&segment, 1, false)) {
            return 2;
        }
        stream->flush();
    }

    // Scripts
    {
        result = WriteScript(stream);
        if (result != 0) {
            return 3;
        }

        segment = {gHeaderType_Script, static_cast<std::size_t>(stream->getDataSize())};
        if (!stream->writeS(&segment, 1, false)) {
            return 2;
        }
        stream->flush();
    }

    segment = {gHeaderType_None, 0};
    if (!stream->writeS(&segment, 1, false)) {
        return 3;
    }
    
    return result;
}

int gModel3D::WriteGameObject(Stream *stream, const GameObject *gameObject) {
    auto name = gameObject->getName();
    auto nsize = name.size();

    // write-name
    {
        stream->writeS(&nsize, 1);

        stream->writeS(name.data(), name.size());
    }
    name = gameObject->getTag();
    nsize = name.size();

    // write-tag
    {
        stream->writeS(&nsize, 1);

        stream->writeS(name.data(), name.size());
    }

    auto bActive = gameObject->getActive();
    auto bEnable = gameObject->getEnable();

    // write-status
    {
        stream->writeS(&bActive, 1);
        stream->writeS(&bEnable, 1);
    }

    // Temp
    auto entt = gameObject->getEntity();

    for (auto &&[cname, ptr] : entt->getComponents()) {
        m_componentTemp.emplace_front(gameObject->getName(), cname, &(*ptr));
    }

    /*
    auto csize = m_temp.size();
    stream->writeS(&csize, 1);

    for (auto &&[cname, ptr] : m_temp) {
        auto cnsize = cname.size(); 
        
        stream->writeS(&cnsize, 1);
        stream->writeS(cname.data(), cnsize);

        ptr->Serialize(stream);
    }
    */

    auto children = gameObject->getTransform()->getChildren();
    auto children_size = children.size();

    stream->writeS(&children_size, 1);
    
    for (auto &child : children) {
        gameObject = child->getGameObject();

        WriteGameObject(stream, gameObject);
    }
    return 0;
}

int gModel3D::WriteScript(Stream *stream) {
    auto size = m_componentTemp.size();

    if (!stream->writeS(&size, 1)) {
        return 1;
    }

    for (auto &&[object, component, ptr] : m_componentTemp) {
        auto nsize = object.size();

        // GameObject - name
        {
            if (!stream->writeS(&nsize, 1)) {
                return 2;
            }
            
            if (!stream->writeS(object.data(), nsize)) {
                return 3;
            }
        }

        // GameObject - ScriptName
        {
            nsize = component.size();
            if (!stream->writeS(&nsize, 1)) {
                return 4;
            }
            
            if (!stream->writeS(component.data(), nsize)) {
                return 5;
            }
        }

        Stream temp;
        ptr->Serialize(&temp);

        std::size_t data_size = temp.getDataSize();

        if (!stream->writeS(&data_size, 1)) {
            temp.clear();
            return 6;
        }
        ptr->Serialize(stream);

        temp.clear();
    }
    return 0;
}

void gModel3D::SetupScene(std::shared_ptr<FbxScene> scene) {
    auto& fRootNode = scene->getRootNode();
    
    for (const auto& node : fRootNode.getChildren()) {
        WriteHeader(&(*scene), &node);
    }
}

void gModel3D::WriteHeader(const FbxScene *scene, const FbxNode *node) {
    LOG_INFO_ARGS("Write 3D Mode: %s", node->getName().c_str());

    Stream stream(File::GetAsset("/model/" + node->getName() + ".gmd"), std::ios_base::out | std::ios_base::binary);

    gHeader pHeader {{'g', 'r', 'm', 'd'}, 1.0f};
    if (!stream.writeS(&pHeader, 1, false)) {
        return LOG_ERROR("Failed to write the 3D model (0)");
    }

    WriteNode(scene, node, &stream);
}

/*
* BinStruct:
* FileHeader->
*               Structs:
*                   GameObject
*                   Script
*/

void gModel3D::WriteNode(const FbxScene *scene, const FbxNode *node, Stream *stream) {
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


