#include "terrain.hpp"

#include "meshFilter.h"
#include "MeshRender.h"
#include "Transform.h"

#include "../simplexNoise/SimplexNoise.h"

void Terrain::OnInit() {
    entity->addComponent<MeshFilter>();
    entity->addComponent<MeshRender>();

    build();
}

void Terrain::build() {
    auto& meshFilter = entity->getComponent<MeshFilter>();

    auto& transform = entity->getComponent<Transform>();
    transform.scale = {30.0f, 1.0f, 20.0f};

    auto& mesh = meshFilter.getMesh();

    u16 TerrainSize = 100;

    for (u16 x=0;x<TerrainSize;x++) {
        for (u16 z=0;z<TerrainSize;z++) {
            float height = SimplexNoise::noise((x + transform.position.x) * 0.005f, (z + transform.position.z) * 0.005f);

            Vector3 vertice(float(x) * 2.0f, height, float(z) * 2.0f);

            mesh.addVertice(vertice);
        }
    }

    for (u16 x = 0; x < TerrainSize - 1; x++) {
        for (u16 z = 0; z < TerrainSize - 1; z++) {
            // Índices dos vértices dos quadrados
            u32 index0 = x * TerrainSize + z;
            u32 index1 = (x + 1) * TerrainSize + z;
            u32 index2 = x * TerrainSize + (z + 1);
            u32 index3 = (x + 1) * TerrainSize + (z + 1);

            // Triângulo 1 (index0, index1, index2)
            mesh.addTriangle(index0, index1, index2);

            // Triângulo 2 (index1, index3, index2)
            mesh.addTriangle(index0, index2, index3);
        }
    }
}
