#pragma once

#include "../ecs/component.h"

#include "../geometry/mesh.h"
#include "../geometry/boundbox.h"

class MeshFilter : public Component {
public:
    Mesh &getMesh();

    void processBoundbox();

    Boundbox &getBoundbox();
private:
    Mesh m_mesh;

    Boundbox m_boundbox;
};

