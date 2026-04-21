#include "gr-engine/task/filmboxtask.hpp"

#include "gr-base/core/assetReference.hpp"

#include "gr-base/ecs/componentstorage.hpp"

#include "gr-engine/filmbox/filmbox.hpp"

#include "gr-engine/managers/viewmanager.hpp"

#include "gr-engine/view/popup/filmboximport.hpp"

#include <cstring>
#include <gr-base/core/file/memoryStream.hpp>
#include <gr-base/core/file/file.hpp>

#include <gr-base/core/gameObject.hpp>

#include <gr-base/core/console.hpp>

#include <gr-base/script/skinnedMeshRender.hpp>
#include <gr-base/script/meshRender.hpp>
#include <gr-base/script/meshFilter.hpp>
#include <gr-base/script/transform.hpp>
// #include <gr-base/script/animator.hpp>

#include <gr-base/managers/assetsManager.hpp>

FilmboxLoadTask::FilmboxLoadTask(const std::string& file) : m_file(file), m_scene(nullptr), m_shader("shader/default")
{}

void FilmboxLoadTask::onPreExecute()
{
    Console::LogInfo("[ FilmboxTask ] Loading: %s", m_file.c_str());
}

int FilmboxLoadTask::doInBackground()
{
    Filmbox filmbox;
    try
    {
        filmbox.load(m_file);
    }
    catch (const std::exception &e)
    {
        return 1;
    }

    m_scene = filmbox.getScene();

    for (auto& fbxMaterial : m_scene->materials)
    {
        if (m_materialCacheList.find(fbxMaterial.name) != m_materialCacheList.end())
            continue;

        MaterialAsset asset;
        asset.SetName(fbxMaterial.name);
        asset.SetShader(m_shader);

        asset.Set("albedo", fbxMaterial.albedo);
        asset.Set("metallic", fbxMaterial.metallic);
        asset.Set("roughness", fbxMaterial.roughness);
        asset.Set("ambientocclusion", fbxMaterial.ao);

        m_materialCacheList.emplace(fbxMaterial.name, asset);
    }

    auto& root = m_scene->getRootNode();
    for (auto& fbxNode : root.getChildren())
        m_objectCacheList.push_back(foreach_node(&fbxNode));

    return 0;
}

void FilmboxLoadTask::onPostExecute(int result)
{
    if (result != 0)
        return Console::LogError("[FilmboxLoadTask] fail in to load '%s' error code: %d", m_file.c_str(), result);

    FilmboxImport* view = ViewManager::GetView<FilmboxImport>();
    if (view == nullptr)
        view = ViewManager::AddSubView<FilmboxImport>();

    auto name = gfile::getname(m_file.c_str());

    ModelCache cache;
    cache.materialsAsset = m_materialCacheList;
    cache.meshesAsset = m_meshCacheList;

    GameObjectHandle handle = GameObject::Create(gfile::getname(m_file.c_str()));

    for (auto &objectcache : m_objectCacheList)
    {
        CreateGameObjectFromCache(&objectcache, handle);
    }

    cache.modelAsset = ConvertToAssetModel(handle);

    GameObject::DestroyImediate(handle);

    view->addFile(name, cache);
}

FilmboxLoadTask::GameObjectCache FilmboxLoadTask::foreach_node(const FbxNode* node)
{
    std::string SelfMeshName;
    for (auto& meshID : node->GetMesheIDs())
    {
        auto& fbxMesh = m_scene->meshes[meshID];
        const auto& meshName = fbxMesh.GetName();

        SelfMeshName = meshName;

        auto& asset = m_meshCacheList[meshName]; 

        asset.vertices.reserve(asset.vertices.size() + fbxMesh.vertices.size());
        
        for (const auto& v : fbxMesh.vertices)
        {
            asset.vertices.push_back({v.position, v.normal, v.texCoord});
        }

        asset.submeshes.push_back(SubMesh{fbxMesh.indices, m_scene->materials[fbxMesh.GetMaterialID()].name});
    }

    GameObjectCache cache;
    cache.name = node->getName();
    cache.position = node->getPosition();
    cache.rotation = node->getRotation();
    cache.scale = node->getScale();
    cache.meshName = SelfMeshName;

    for (auto& child : node->getChildren())
        cache.childrent.push_back(foreach_node(&child));

    return cache;
}


GameObjectHandle FilmboxLoadTask::CreateGameObjectFromCache(GameObjectCache* cache, GameObjectHandle parent)
{
    GameObjectHandle handle = GameObject::Create(cache->name);

    GameObject* gameObject = GameObject::Get(handle);
    gameObject->SetIsPrefab(true);

    auto transform = gameObject->GetTransform();
    transform->setLocalPosition(cache->position);
    transform->setLocalRotation(cache->rotation);
    transform->setLocalScale(cache->scale);

    if (!cache->meshName.empty())
    {
        auto& asset = m_meshCacheList[cache->meshName];

        std::vector<MaterialID> materials;
        for(auto& sub : asset.submeshes)
        {
            auto& material = m_materialCacheList[sub.materialName];

            auto materialID = AssetsManager::FindMaterial(sub.materialName);
            if (materialID == ID_INVALID)
                materialID = AssetsManager::CreateMaterial(sub.materialName);

            materials.push_back(materialID);
        }

        // MeshID meshID = AssetsManager::FindMesh(cache->meshName);
        // if (meshID == ID_INVALID)
        //     meshID = AssetsManager::CreateMesh(cache->meshName, &asset);
        //
        // auto meshFilter = gameObject->addComponent<MeshFilter>();
        // meshFilter->SetMesh(meshID);
        //
        auto meshRender = gameObject->addComponent<MeshRender>();
        meshRender->SetMaterial(materials);
    }

    if (parent != HANDLE_INVALID)
    {
        GameObject* gameObjectParent = GameObject::Get(parent);

        auto transformParent = gameObjectParent->GetTransform();
        transformParent->addChildren(gameObject->GetTransform());
    }

    for (auto& child : cache->childrent)
        CreateGameObjectFromCache(&child, handle);

    return handle;
}


ModelAsset FilmboxLoadTask::ConvertToAssetModel(GameObjectHandle handle)
{
    GameObject* gameObject = GameObject::Get(handle);

    ModelAsset asset;
    asset.name = gameObject->GetName();
    asset.isStatic = gameObject->GetIsStatic();

    for (auto& hash : gameObject->GetHashes())
    {
        Component* behavior = gameObject->getComponent(hash);

        MemoryStream stream;
        behavior->Serialize(&stream);

        std::vector<uint8_t> buffer(stream.tellg(), 0);
        memcpy(buffer.data(), stream.data(), stream.tellg());

        asset.componentBuffer.emplace(hash, buffer);
    }

    auto transform = gameObject->GetTransform();
    for (auto& childID : transform->getChildren())
    {
        auto childTransform = ComponentStorage::GetComponent<Transform>(childID);

        asset.childrent.push_back(ConvertToAssetModel(childTransform->GetGameObject()->GetCacheID()));
    }

    return asset;
}


