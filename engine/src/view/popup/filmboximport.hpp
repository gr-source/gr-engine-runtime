#pragma once

#include "ui/viewlayout.hpp"

#include <gr-base/ecs/default.hpp>

class FilmboxImport : public ViewLayout
{
public:
    FilmboxImport(GameObject* gameObject);

private:
    GameObject* m_gameObject;

    void onRenderUI() override;

    void render_tree_node(const GameObject *gameObject);


    void onSavedGameObject(GameObject *gameObject);
};


