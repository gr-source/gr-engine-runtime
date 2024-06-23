#pragma once

#include "../ecs/entity.h"

#include "../geometry/boundBox.h"
#include "../geometry/mesh.h"

#include "../bufferManager.h"

class MeshFilter : public Component {
public:
    MeshFilter();

    Mesh& getMesh();
private:
    Mesh m_mesh;
};