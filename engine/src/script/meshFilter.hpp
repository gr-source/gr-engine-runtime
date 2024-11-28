#pragma once

#include "geometry/mesh.hpp"
#include "geometry/boundbox.hpp"

#include <ecs/component.hpp>

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

