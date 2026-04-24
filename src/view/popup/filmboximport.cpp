#include "gr-engine/view/popup/filmboximport.hpp"

#include "gr-base/core/assetReference.hpp"
#include "gr-base/core/file/file.hpp"

#include "gr-base/core/thread/taskmanager.hpp"

#include "gr-base/managers/assetsManager.hpp"
#include "gr-base/managers/eventManager.hpp"
#include "gr-engine/managers/viewmanager.hpp"

#include <gr-base/script/transform.hpp>

#include <gr-base/core/gameObject.hpp>

#include "gr-base/task/materialsavetask.hpp"
#include "gr-base/task/modelsavetask.hpp"
#include "gr-base/task/meshsavetask.hpp"

#include "imgui.h"

#include <imgui.h>

FilmboxImport::FilmboxImport()
{}

void FilmboxImport::addFile(const std::string& name, const ModelCache& cache)
{
    m_modelsCaching.emplace(name, cache);
}

void FilmboxImport::onRenderUI()
{
    for (auto &&[name, cache] : m_modelsCaching)
    {
        if (ImGui::CollapsingHeader(name.c_str()))
        {
            auto& materialsCache = cache.materialsAsset;
            if (!materialsCache.empty() && ImGui::TreeNode("Material"))
            {
                for (auto&&[name, asset] : materialsCache)
                {
                    ImGui::Text("%s", name.c_str());
                }
                ImGui::TreePop();
            }

            auto& meshesCache = cache.meshesAsset;
            if (!meshesCache.empty() && ImGui::TreeNode("Mesh"))
            {
                for (auto &&[name, mesh] : meshesCache)
                    ImGui::Text("%s sub: %ld", name.c_str(), mesh.submeshes.size());

                ImGui::TreePop();
            }

            if (ImGui::TreeNode("GameObject"))
            {
                render_tree_node(&cache.modelAsset);
                ImGui::TreePop();
            }
        }
    }

    auto avail = ImGui::GetContentRegionAvail();

    if (ImGui::Button("Save", ImVec2(avail.x, 0.0f)))
    {
        auto dir = AssetsManager::GetAssetDir();

        for (auto &&[name, cache] : m_modelsCaching)
        {
            // material
            {
                auto newdirmat = dir + "/material/" + name;
                if (!gfile::exists(newdirmat))
                    gfile::create_directory(newdirmat);

                for (auto&&[name, asset] : cache.materialsAsset)
                {
                    auto path = newdirmat + "/" + name + ".gmat";

                    AssetReference::Registry(name, path);

                    TaskManager::Add<MaterialSaveTask>(path, asset);
                }
            }

            // mesh
            {
                auto newdirmat = dir + "/mesh/" + name;
                if (!gfile::exists(newdirmat))
                    gfile::create_directory(newdirmat);

                for (auto&&[name, asset] : cache.meshesAsset)
                {
                    TaskManager::Add<MeshSaveTask>(newdirmat + "/" + name + ".gmsh", asset);
                }
            }

            // gameObject
            {
                auto newdirmat = dir + "/model/" + name;
                if (!gfile::exists(newdirmat))
                    gfile::create_directory(newdirmat);

                auto& asset = cache.modelAsset;

                TaskManager::Add<ModelSaveTask>(newdirmat + "/" + asset.name + ".gmdl", asset);
            }
        }

        ViewManager::RemoveView<FilmboxImport>();
    }
}

void FilmboxImport::render_tree_node(const ModelAsset* asset)
{
    auto& childrent = asset->childrent;

    auto isOpen = ImGui::TreeNodeEx(asset->name.c_str(), childrent.empty() ? ImGuiTreeNodeFlags_Leaf : 0);
    if (!isOpen)
        return;

    for (auto &child : childrent)
        render_tree_node(&child);

    ImGui::TreePop();
}



