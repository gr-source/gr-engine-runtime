#pragma once

#include "gr-base/asset/materialasset.hpp"
#include "gr-base/asset/meshasset.hpp"

#include "gr-base/asset/modelasset.hpp"
#include "gr-engine/ui/viewlayout.hpp"

#include <gr-base/core/geometry/default.hpp>

#include <gr-base/ecs/default.hpp>

#include <string>

#include <unordered_map>

struct ModelCache
{
    std::unordered_map<std::string, MaterialAsset> materialsAsset;
    std::unordered_map<std::string, MeshAsset> meshesAsset;

    ModelAsset modelAsset;
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

    void render_tree_node(const ModelAsset *asset);
};


