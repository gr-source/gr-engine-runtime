#pragma once

#include "../ecs/component.h"

#include "../geometry/mesh.h"
#include "../geometry/boundbox.h"

class MeshFilter : public Component {
public:
    MeshFilter();

    void setMesh(Mesh *mesh);

    Mesh *getMesh();

    void processBoundbox();

    Boundbox &getBoundbox();
private:
    Mesh *m_sharedMesh;

    Boundbox m_boundbox;
};

