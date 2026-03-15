#pragma once

#include "gr-engine/ui/viewlayout.hpp"

#include <gr-base/core/geometry/default.hpp>

#include <gr-base/ecs/default.hpp>

#include <vector>

class FilmboxImport : public ViewLayout
{
public:
    FilmboxImport(GameObjectHandle gameObjectID, std::vector<MeshID> meshes);

private:
    std::vector<MeshID> m_meshes;
    
    GameObjectHandle m_gameObjectID;

    void onRenderUI() override;

    void render_tree_node(const GameObject *gameObject);

    void onSavedGameObject(GameObjectHandle gameObject);
};


