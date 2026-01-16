#pragma once

#include "ui/viewlayout.hpp"

#include <gr-base/core/default.hpp>

class FilmboxImport : public ViewLayout
{
public:
    FilmboxImport(GameObjectID gameObjectID);

private:
    GameObjectID m_gameObjectID;

    void onRenderUI() override;

    void render_tree_node(const GameObject *gameObject);

    void onSavedGameObject(GameObjectID gameObject);
};


