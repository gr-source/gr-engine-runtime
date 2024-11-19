#pragma once

#include <gr-render/gRender.h>

#include "default.hpp"

#include "material.hpp"
#include "boundbox.hpp"

class Batching {
public:
    enum BatchingType {
        BatchingType_Dynamic           = 0,
        BatchingType_Static_Mesh       = 1,
        BatchingType_Instanced_Dynamic = 2,
        BatchingType_Instanced_Static  = 3
    };

    static void Initialize();

    static void Shutdown();

    static void RenderShadowPass();
    
    static grm::size GetBatchCount();

    static Batching *FindBatchByName(const std::string &name);

    static void RemoveBatchByName(const std::string &name);

    template <typename ...Args>
    static Batching &AddBatch(const std::string &key, Args &&...args);

    static void RenderBatches();
    
    Batching(const std::string &name, std::vector<std::shared_ptr<Material>> *materials, Mesh *mesh, grr::gShader *shader, int flags);
    ~Batching();

    void insert(Transform *transform);

    void remove(Transform *transform, bool bDelete = true);

    void draw(grr::gShader *shader = nullptr, bool bSetMaterial = true);

    bool hasModel() const;
    
    grm::u32 m_instanceCount;
    
private:
    enum DrawType {
        DrawType_Elements_Instanced = 1 << 1,
        DrawType_Arrays_Instanced   = 1 << 2,
        DrawType_Elements           = 1 << 3,
        DrawType_Arrays             = 1 << 4
    };

    static std::unordered_map<std::string, std::unique_ptr<Batching>> s_batchedInstances;

    static std::vector<Batching *> s_batchedInstancesList;

    static grr::gShader *s_shadowShader;

    std::vector<std::shared_ptr<Material>> *m_materials;

    std::vector<Transform *> m_models;

    grr::gVertexArray *m_vertexArray;

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

    void BuildBatch(Mesh *mesh);

    void CompileBatch();

    void buildDynamic();

    void buildDynamicInstanced();

    void buildStaticInstanced();

    void buildStaticMesh();

    void setupMaterial();

    static void RegisterShaderShadow(grr::gShader *shader);
};

template <typename... Args>
inline Batching &Batching::AddBatch(const std::string &key, Args &&...args) {
    auto &&[it, added] = s_batchedInstances.emplace(key, std::make_unique<Batching>(key, std::forward<Args>(args)...));
    assert(added && "Value subscrite.");
    
    auto ptr =  &(*it->second);
    s_batchedInstancesList.push_back(ptr);
    return *ptr;
}
