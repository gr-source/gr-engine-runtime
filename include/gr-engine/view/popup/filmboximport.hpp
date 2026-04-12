#pragma once

#include "gr-base/core/geometry/materialAsset.hpp"
#include "gr-base/core/geometry/meshAsset.hpp"

#include "gr-engine/ui/viewlayout.hpp"

#include <gr-base/core/geometry/default.hpp>

#include <gr-base/ecs/default.hpp>

#include <string>
#include <unordered_set>
#include <vector>

struct ModelCache
{
    std::unordered_map<std::string, MaterialAsset> materialsAsset;
    std::unordered_map<std::string, MeshAsset> meshesAsset;
};

class FilmboxImport : public ViewLayout
{
public:
    FilmboxImport();

    inline const char* getName() const override
    {
        return "Filmbox Import";
    }

    void addFile(const std::string& name, const ModelCache& cache);

private:
    std::unordered_map<std::string, ModelCache> m_modelsCaching;

    void onRenderUI() override;

    void render_tree_node(const GameObject *gameObject);

    void onSavedGameObject(GameObjectHandle gameObject);
};


