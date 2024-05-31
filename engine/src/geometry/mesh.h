#pragma once

#include <vector>

#include "../common.h"

#include "vertex3D.h"

class Mesh {
public:
    Mesh(const std::string& name = "unnamed");

    template <typename ...Args>
    Vertex3D& addvertex(Args&& ...args);

    const std::vector<Vertex3D>& getvertices() const;

    Vertex3D& GetVertex(u32 index);

    const u32 getverticecount() const;

    const Vertex3D* getverticedata() const;

    void addindice(u32 indice);

    void addindex(u32 indice);

    void addtriangle(u32 t0, u32 t1, u32 t2);

    const std::vector<u32>& getindices() const;

    const u32 getelementcount() const;

    const u32* getelementdata() const;

    u32& GetIndice(u32 index);

    const bool hasindices() const;

    void setname(const std::string& name);

    const std::string& getname() const;

    bool operator ==(const Mesh& other) const;

    void addMaterial(u32 index);

    const std::vector<u32>& getMaterials() const;

    u32* findMaterial(u32 index);

protected:
    std::string name;

    std::vector<Vertex3D> vertices;

    std::vector<u32> indices;

    std::vector<u32> m_material;

    friend class Filmbox;
    friend class MeshRender;
};

template <typename ...Args>
inline Vertex3D& Mesh::addvertex(Args&& ...args) {
    return vertices.emplace_back(std::forward<Args>(args)...);
}
