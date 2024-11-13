#pragma once

#include "../geometry/mesh.h"
#include "../geometry/boundbox.h"

#include "ecs/component.hpp"

class MeshFilter : public Component {
public:
    MeshFilter() = delete;

    void setMesh(Mesh *mesh);

    Mesh *getMesh();

    void processBoundbox();

    Boundbox &getBoundbox();
    
private:
    Mesh *m_sharedMesh;

    Boundbox m_boundbox;
};

