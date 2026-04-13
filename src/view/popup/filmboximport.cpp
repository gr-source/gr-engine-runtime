#include "gr-engine/view/popup/filmboximport.hpp"

#include "gr-base/core/file/file.hpp"
#include "gr-base/core/task/runtime/gameobjectsavetask.hpp"
#include "gr-base/core/task/runtime/meshSaveTask.hpp"

#include "gr-base/core/thread/taskmanager.hpp"

#include "gr-base/ecs/behaviorStorage.hpp"
#include "gr-base/managers/assetsManager.hpp"
#include "gr-base/managers/eventManager.hpp"
#include "gr-engine/managers/viewmanager.hpp"

#include <gr-base/script/transform.hpp>

#include <gr-base/core/gameObject.hpp>

#include "gr-engine/engine.h"
#include "gr-engine/task/materialsavetask.hpp"
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
                    ImGui::Text("%s", name.c_str());

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
                    TaskManager::Add<MaterialSaveTask>(newdirmat + "/" + name + ".gmat", asset);
                }
            }
        }

        ViewManager::RemoveView<FilmboxImport>();
    }
}

void FilmboxImport::render_tree_node(const GameObject *gameObject)
{
    auto transform = gameObject->GetTransform();

    auto isOpen = ImGui::TreeNodeEx(gameObject->GetName().c_str(), !transform->hasChildren() ? ImGuiTreeNodeFlags_Leaf : 0);
    if (!isOpen)
        return;

    for (auto &child : transform->getChildren())
        render_tree_node(BehaviorStorage::GetComponent<Transform>(child)->GetGameObject());

    ImGui::TreePop();
}



