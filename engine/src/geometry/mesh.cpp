#include "mesh.h"

#include "../filesystem/stream.h"

Mesh::Mesh(const std::string& name) : name(name) {}

const std::vector<Vertex3D>& Mesh::getvertices() const {
    return vertices;
}

Vertex3D& Mesh::GetVertex(u32 index) {
    return vertices[index];
}

const u32 Mesh::getverticecount() const {
    return vertices.size();
}

const Vertex3D *Mesh::getverticedata() const {
    return vertices.data();
}

void Mesh::addindice(u32 indice) {
    indices.emplace_back(indice);
}

void Mesh::addindex(u32 indice) {
     indices.emplace_back(indice);
}

void Mesh::addtriangle(u32 t0, u32 t1, u32 t2) {
    indices.emplace_back(t0);
    indices.emplace_back(t1);
    indices.emplace_back(t2);
}

const std::vector<u32>& Mesh::getindices() const {
    return indices;
}

const u32 Mesh::getelementcount() const {
    return indices.size();
}

const u32 *Mesh::getelementdata() const {
    return indices.data();
}

u32& Mesh::GetIndice(u32 index) {
    return indices[index];
}

const bool Mesh::hasindices() const {
    return !indices.empty();
}

void Mesh::setname(const std::string& name) {
    this->name = name;
}

const std::string& Mesh::getname() const {
    return name;
}

const Skeleton &Mesh::getskeleton() const {
    return m_skeleton;
}

const bool Mesh::hasBones() const {
    return m_skeleton.hasbone();
}

bool Mesh::operator ==(const Mesh& other) const {
    return this == &other;
}

/*
Mesh Mesh::CreateCube() {
    Mesh m_mesh("Cube");

    Stream::open("cube.obj", std::ios_base::in);

    std::vector<Vector3> vertices;
    std::vector<Vector3> normal;
    std::vector<Vector3> uv;

    std::string line;
    while (Stream::getline(line)) {
        std::stringstream ss(line);

        std::string key;

        if (!(ss >> key))
            continue;
        
        if (key == "v") {
            Vector3 v;
            if (!(ss >> v)) continue;

            vertices.push_back(v);
        } else if (key == "vn") {
            Vector3 n;
            if (!(ss >> n)) continue;

            normal.push_back(n);
        } else if (key == "vt") {
            Vector2 t;
            if (!(ss >> t)) continue;

            uv.push_back(t);
        } else if (key == "f") {
            u32 v0, vt0, vn0;
            char c;

            for (int i=0;i<2;i++) {
                if (!(ss >> v0 >> c >> vt >> v >> vn0)) continue;


            }
        }
    }
    Stream::close();

    return m_mesh;
}
*/

