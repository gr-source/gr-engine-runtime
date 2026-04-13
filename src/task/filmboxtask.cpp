#include "gr-engine/task/filmboxtask.hpp"

#include "gr-engine/filmbox/filmbox.hpp"

#include "gr-engine/managers/viewmanager.hpp"

#include "gr-engine/view/popup/filmboximport.hpp"

#include <gr-base/core/file/file.hpp>

#include <gr-base/core/gameObject.hpp>

#include <gr-base/core/console.hpp>

#include <gr-base/script/skinnedMeshRender.hpp>
#include <gr-base/script/meshRender.hpp>
#include <gr-base/script/meshFilter.hpp>
#include <gr-base/script/transform.hpp>
// #include <gr-base/script/animator.hpp>

#include <gr-base/managers/assetsManager.hpp>
#include <utility>

FilmboxLoadTask::FilmboxLoadTask(const std::string& file) : m_file(file)
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

    auto scene = std::move(filmbox.get_scene());

    for (auto& fbxMaterial : scene->materials)
    {
        if (m_materialCacheList.find(fbxMaterial.name) != m_materialCacheList.end())
            continue;

        MaterialAsset asset;
        asset.Set("albedo", fbxMaterial.albedo);
        asset.Set("metallic", fbxMaterial.metallic);
        asset.Set("roughness", fbxMaterial.roughness);
        asset.Set("ambientocclusion", fbxMaterial.ao);

        m_materialCacheList.emplace(fbxMaterial.name, asset);
    }

    for (auto& fbxMesh : scene->meshes)
    {
        if (m_meshCacheList.find(fbxMesh.GetName()) != m_meshCacheList.end())
            continue;

        MeshAsset asset;

        for (auto &v : fbxMesh.vertices)
            asset.vertices.push_back({v.position, v.normal, v.texCoord});

        asset.indices = fbxMesh.indices;

        m_meshCacheList.emplace(fbxMesh.GetName(), asset);
    }

    auto& root = scene->getRootNode();

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

    view->addFile(name, cache);
}




