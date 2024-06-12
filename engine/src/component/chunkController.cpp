#include "chunkController.h"

#include "MeshRender.h"
#include "Transform.h"

SimplexNoise ChunkController::Noise;

int ChunkController::ChunkSize = 16;

ChunkController::ChunkController() {
}

void ChunkController::OnInit() {
    if (!entity->hasComponent<MeshRender>()) {
        entity->addComponent<MeshRender>();
    }
    BuildMesh();
}

void ChunkController::BuildMesh() {
    auto& transform = entity->getComponent<Transform>(); // chunck

    for (int x = 0; x < ChunkSize; x++) {
        for (int y = 0; y < ChunkSize; y++) {
            for (int z = 0; z < ChunkSize; z++) {
                auto height = SimplexNoise::noise(x, y, z);

                m_blocks[x][y][z].isExists = height > 0;
                m_blocks[x][y][z].transform.position = {(float)x, (float)y, (float)z};
            }
        }
    }



    auto& meshRender = entity->getComponent<MeshRender>();
    auto& mesh = meshRender.getMesh();
    mesh.clear();

    for (int x = 0; x < ChunkSize; x++) {
        for (int y = 0; y < ChunkSize; y++) {
            for (int z = 0; z < ChunkSize; z++) {
                auto& block = m_blocks[x][y][z];

                // verifica se o bloco atual existe
                if (block.isExists) {
                    auto& model = block.transform.getglobalmatrix();

                    // verifica se existe o bloco acima existe se for o caso não adiciona um face.
                    // Vericica se o bloco acima esta na borda da chunck se tiver adiciona uma face

                    // Top face
                    if (!m_blocks[x][y + 1][z].isExists || x < 0 || x >= ChunkSize || (y + 1) < 0 || (y + 1) >= ChunkSize || z < 0 || z >= ChunkSize) {
                        mesh.addvertex(Vertex3D(Vector3(model * Vector4(-1.0f, 1.0f, -1.0f, 1.0f)), Vector3(0.0f, 1.0f, 0.0f)));
                        mesh.addvertex(Vertex3D(Vector3(model * Vector4( 1.0f, 1.0f, -1.0f, 1.0f)), Vector3(0.0f, 1.0f, 0.0f)));
                        mesh.addvertex(Vertex3D(Vector3(model * Vector4( 1.0f, 1.0f,  1.0f, 1.0f)), Vector3(0.0f, 1.0f, 0.0f)));
                        mesh.addvertex(Vertex3D(Vector3(model * Vector4(-1.0f, 1.0f,  1.0f, 1.0f)), Vector3(0.0f, 1.0f, 0.0f)));

                        u32 baseIndex = mesh.getverticecount() - 4;
                        mesh.addtriangle(baseIndex, baseIndex + 2, baseIndex + 1);
                        mesh.addtriangle(baseIndex, baseIndex + 3, baseIndex + 2);
                    }

                    // Bottom face
                    if (y - 1 > 0 && !m_blocks[x][y - 1][z].isExists || x < 0 || x >= ChunkSize || (y - 1) < 0 || (y - 1) >= ChunkSize || z < 0 || z >= ChunkSize) {
                        mesh.addvertex(Vertex3D(Vector3(model * Vector4(-1.0f, -1.0f,  1.0f, 1.0f)), Vector3(0.0f, -1.0f, 0.0f)));
                        mesh.addvertex(Vertex3D(Vector3(model * Vector4( 1.0f, -1.0f,  1.0f, 1.0f)), Vector3(0.0f, -1.0f, 0.0f)));
                        mesh.addvertex(Vertex3D(Vector3(model * Vector4( 1.0f, -1.0f, -1.0f, 1.0f)), Vector3(0.0f, -1.0f, 0.0f)));
                        mesh.addvertex(Vertex3D(Vector3(model * Vector4(-1.0f, -1.0f, -1.0f, 1.0f)), Vector3(0.0f, -1.0f, 0.0f)));

                        u32 baseIndex = mesh.getverticecount() - 4;
                        mesh.addtriangle(baseIndex, baseIndex + 2, baseIndex + 1);
                        mesh.addtriangle(baseIndex, baseIndex + 3, baseIndex + 2);
                    }

                    // Front face
                    if (!m_blocks[x + 1][y][z].isExists || (x + 1) < 0 || (x + 1) >= ChunkSize || y < 0 || y >= ChunkSize || z < 0 || z >= ChunkSize) {
                        mesh.addvertex(Vertex3D(Vector3(model * Vector4(1.0f, -1.0f, -1.0f, 1.0f)), Vector3(1.0f, 0.0f, 0.0f)));
                        mesh.addvertex(Vertex3D(Vector3(model * Vector4(1.0f,  1.0f, -1.0f, 1.0f)), Vector3(1.0f, 0.0f, 0.0f)));
                        mesh.addvertex(Vertex3D(Vector3(model * Vector4(1.0f,  1.0f,  1.0f, 1.0f)), Vector3(1.0f, 0.0f, 0.0f)));
                        mesh.addvertex(Vertex3D(Vector3(model * Vector4(1.0f, -1.0f,  1.0f, 1.0f)), Vector3(1.0f, 0.0f, 0.0f)));

                        u32 baseIndex = mesh.getverticecount() - 4;
                        mesh.addtriangle(baseIndex, baseIndex + 1, baseIndex + 2);
                        mesh.addtriangle(baseIndex, baseIndex + 2, baseIndex + 3);
                    }

                    // Back face
                    if (x - 1 > 0 && !m_blocks[x - 1][y][z].isExists || (x - 1) < 0 || (x - 1) >= ChunkSize || y < 0 || y >= ChunkSize || z < 0 || z >= ChunkSize) {
                        mesh.addvertex(Vertex3D(Vector3(model * Vector4(-1.0f, -1.0f, -1.0f, 1.0f)), Vector3(-1.0f, 0.0f, 0.0f)));
                        mesh.addvertex(Vertex3D(Vector3(model * Vector4(-1.0f,  1.0f, -1.0f, 1.0f)), Vector3(-1.0f, 0.0f, 0.0f)));
                        mesh.addvertex(Vertex3D(Vector3(model * Vector4(-1.0f,  1.0f,  1.0f, 1.0f)), Vector3(-1.0f, 0.0f, 0.0f)));
                        mesh.addvertex(Vertex3D(Vector3(model * Vector4(-1.0f, -1.0f,  1.0f, 1.0f)), Vector3(-1.0f, 0.0f, 0.0f)));

                        u32 baseIndex = mesh.getverticecount() - 4;
                        mesh.addtriangle(baseIndex, baseIndex + 2, baseIndex + 1);
                        mesh.addtriangle(baseIndex, baseIndex + 3, baseIndex + 2);
                    }

                    // Right face
                    if (!m_blocks[x][y][z + 1].isExists || x < 0 || x >= ChunkSize || y < 0 || y >= ChunkSize || (z + 1) < 0 || (z + 1) >= ChunkSize) {
                        mesh.addvertex(Vertex3D(Vector3(model * Vector4( 1.0f, -1.0f, 1.0f, 1.0f)), Vector3(0.0f, 0.0f, 1.0f)));
                        mesh.addvertex(Vertex3D(Vector3(model * Vector4( 1.0f,  1.0f, 1.0f, 1.0f)), Vector3(0.0f, 0.0f, 1.0f)));
                        mesh.addvertex(Vertex3D(Vector3(model * Vector4(-1.0f,  1.0f, 1.0f, 1.0f)), Vector3(0.0f, 0.0f, 1.0f)));
                        mesh.addvertex(Vertex3D(Vector3(model * Vector4(-1.0f, -1.0f, 1.0f, 1.0f)), Vector3(0.0f, 0.0f, 1.0f)));

                        u32 baseIndex = mesh.getverticecount() - 4;
                        mesh.addtriangle(baseIndex, baseIndex + 1, baseIndex + 2);
                        mesh.addtriangle(baseIndex, baseIndex + 2, baseIndex + 3);
                    }

                    // Left face
                    if (z - 1 > 0 && !m_blocks[x][y][z - 1].isExists || x < 0 || x >= ChunkSize || y < 0 || y >= ChunkSize || (z - 1) < 0 || (z - 1) >= ChunkSize) {
                        mesh.addvertex(Vertex3D(Vector3(model * Vector4( 1.0f, -1.0f, -1.0f, 1.0f)), Vector3(0.0f, 0.0f, -1.0f)));
                        mesh.addvertex(Vertex3D(Vector3(model * Vector4( 1.0f,  1.0f, -1.0f, 1.0f)), Vector3(0.0f, 0.0f, -1.0f)));
                        mesh.addvertex(Vertex3D(Vector3(model * Vector4(-1.0f,  1.0f, -1.0f, 1.0f)), Vector3(0.0f, 0.0f, -1.0f)));
                        mesh.addvertex(Vertex3D(Vector3(model * Vector4(-1.0f, -1.0f, -1.0f, 1.0f)), Vector3(0.0f, 0.0f, -1.0f)));

                        u32 baseIndex = mesh.getverticecount() - 4;
                        mesh.addtriangle(baseIndex, baseIndex + 2, baseIndex + 1);
                        mesh.addtriangle(baseIndex, baseIndex + 3, baseIndex + 2);
                    }
                }
            }
        }
    }

    Material m;
    m.albedo = {1.0f, 0.0f, 1.0f, 1.0f};
    m.ao = 1.0f;
    m.roughness = 0.32f;
    m.metallic = 0.2f;

    meshRender.m_materials.emplace(0, m);

    meshRender.buildMesh();
}
