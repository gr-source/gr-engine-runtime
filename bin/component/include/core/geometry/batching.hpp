#pragma once

#include <gr-render/gRender.h>

#include "script/default.hpp"

#include "boundBox.hpp"
#include "material.hpp"

#include <memory>

class Batching {
public:
    Batching() = delete;
    
    enum Batching_Flags {
        Batching_Flags_Dynamic           = 0,
        Batching_Flags_Static_Mesh       = 1,
        Batching_Flags_Instanced_Dynamic = 2,
        Batching_Flags_Instanced_Static  = 3
    };
    
    static void (*InsertInternal)(Batching *instance, Transform *transform);

    static void (*RemoveInternal)(Batching *instance, Transform *transform, bool bDelete);

    void insert(Transform *transform);

    void remove(Transform *transform, bool bDelete = true);
    
private:
    enum DrawType {
        DrawType_Elements_Instanced = 1 << 1,
        DrawType_Arrays_Instanced   = 1 << 2,
        DrawType_Elements           = 1 << 3,
        DrawType_Arrays             = 1 << 4
    };

    std::vector<std::shared_ptr<Material>> *m_materials;

    std::vector<Transform *> m_models;

    grr::gVertexArray *m_vertexArray;

    grm::u32 m_instanceCount;

    grm::u32 m_indexCount;

    std::string m_name;

    Boundbox m_boundbox;

    grm::u32 m_VBO;

    grm::u32 m_EBO;

    grm::u32 m_PBO;

    bool m_hasIndex;

    DrawType m_type;

    Mesh *m_mesh;

    int m_flags;

    grr::gShader *m_shader;
};


