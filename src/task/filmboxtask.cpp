#include "gr-engine/task/filmboxtask.hpp"

#include "gr-base/core/assetReference.hpp"

#include "gr-base/core/scene.hpp"
#include "gr-base/task/modelsavetask.hpp"
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

FilmboxLoadTask::FilmboxLoadTask(const std::string& file) : m_file(file), m_scene(nullptr), m_shader("simple_model.gshd")
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

    auto scene = Scene::GetCurrent();

    auto name = gfile::get_file_name(m_file);

    ModelCache cache;
    cache.materialsAsset = m_materialCacheList;
    cache.meshesAsset = m_meshCacheList;

    Entity entity = scene->create(name.c_str());

    for (auto &objectcache : m_objectCacheList)
    {
        CreateGameObjectFromCache(scene, &objectcache, entity);
    }

    cache.modelAsset = ModelSaveTask::ConvertToAssetModel(scene, entity);

    scene->destroy(entity);

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
        asset.SetName(meshName);

        size_t vertexOffset = asset.vertices.size();

        asset.vertices.reserve(asset.vertices.size() + fbxMesh.vertices.size());
        
        for (const auto& v : fbxMesh.vertices)
        {
            asset.vertices.push_back({v.position, v.normal, v.texCoord});
        }

        auto& submesh = asset.submeshes.emplace_back();
        submesh.materialName = m_scene->materials[fbxMesh.GetMaterialID()].name;

        for (auto& idx : fbxMesh.indices)
        {
            submesh.indices.push_back(idx + static_cast<uint32_t>(vertexOffset));
        }

        // asset.submeshes.push_back(SubMeshAsset{fbxMesh.indices, });
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

Entity FilmboxLoadTask::CreateGameObjectFromCache(Scene* scene, GameObjectCache* cache, Entity parent)
{
    Entity entity = scene->create(cache->name.c_str());

    GameObject gameObject = scene->Wrap(entity);
    // gameObject->SetIsPrefab(true);

    auto transform = gameObject.GetComponent<Transform>();
    transform->position = cache->position;
    transform->rotation = cache->rotation;
    transform->scale = cache->scale;
    transform->dirty = true;

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

        MeshID meshID = AssetsManager::FindMesh(cache->meshName);
        if (meshID == ID_INVALID)
            meshID = AssetsManager::CreateMesh(cache->meshName, &asset);

        auto meshFilter = gameObject.AddComponent<MeshFilter>();
        meshFilter->meshID = meshID;

        auto meshRender = gameObject.AddComponent<MeshRender>();
        for (auto &id : materials)
            meshRender->materials.push_back(id);
    }

    if (parent != HANDLE_INVALID)
    {
        scene->SetParent(parent, entity);
    }

    for (auto& child : cache->childrent)
        CreateGameObjectFromCache(scene, &child, entity);

    return entity;
}




