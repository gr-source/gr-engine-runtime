#pragma once

#include "ui/viewlayout.hpp"

#include <gr-base/core/geometry/default.hpp>

#include <gr-base/core/default.hpp>

#include <vector>

class FilmboxImport : public ViewLayout
{
public:
    FilmboxImport(GameObjectID gameObjectID, std::vector<MeshID> meshes);

private:
    std::vector<MeshID> m_meshes;
    
    GameObjectID m_gameObjectID;

    void onRenderUI() override;

    void render_tree_node(const GameObject *gameObject);

    void onSavedGameObject(GameObjectID gameObject);
};


