#pragma once

#include <vector>

#include "../common.h"

#include "vertex3D.h"

class Mesh {
public:
    Mesh(const std::string& name = "unnamed");

    const std::string& getname() const;

    template <typename ...Args>
    Vertex3D& addVertice(Args&& ...args);

    const std::vector<Vertex3D>& getvertices() const;

    Vertex3D& GetVertex(u32 index);

    const u32 getVerticeCount() const;

    const Vertex3D* getVerticeData() const;

    void addindice(u32 indice);

    void addindex(u32 indice);

    void addTriangle(u32 t0, u32 t1, u32 t2);

    const std::vector<u32>& getindices() const;

    const u32 getIndiceCount() const;

    const u32* getIndiceData() const;

    u32& GetIndice(u32 index);

    const bool hasindices() const;

    bool operator ==(const Mesh& other) const;

    void addMaterial(u32 index);

    const std::vector<u32>& getMaterials() const;

    u32* findMaterial(u32 index);

    template <typename T>
    void insert(const std::vector<T>& vector);

    void clear();

protected:
    std::string m_name;

    std::vector<Vertex3D> m_vertices;

    std::vector<u32> m_material;

    std::vector<u32> m_indices;

    friend class Filmbox;
    friend class MeshRender;
};

template <typename ...Args>
inline Vertex3D& Mesh::addVertice(Args&& ...args) {
    return m_vertices.emplace_back(std::forward<Args>(args)...);
}
