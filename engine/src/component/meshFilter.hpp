#pragma once

#include "../ecs/component.h"

#include "../geometry/mesh.h"

class MeshFilter : public Component {
public:
    Mesh& getMesh();
private:
    Mesh m_mesh;
};

