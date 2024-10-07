#pragma once

#include "../ecs/component.hpp"

#include "core/geometry/boundBox.hpp"
#include "core/geometry/mesh.hpp"

class MeshFilter : public Component {
public:
    MeshFilter();
    
    void setMesh(Mesh *mesh);

    Mesh *getMesh();

    void processBoundbox();

    Boundbox &getBoundbox();
    
    std::ostream &Serialize(std::ostream &os) override;

    std::istream &Deserialize(std::istream &is) override;
    
private:
    Mesh *m_sharedMesh;

    Boundbox m_boundbox;
};

REGISTER_COMPONENT(MeshFilter)
