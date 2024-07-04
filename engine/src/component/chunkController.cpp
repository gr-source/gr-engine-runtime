#include "chunkController.h"

#include "meshFilter.h"
#include "MeshRender.h"
#include "Transform.h"
#include "voxelSettings.hpp"

SimplexNoise ChunkController::Noise;

ChunkController::ChunkController() {}

void ChunkController::OnInit() {
    if (!entity->hasComponent<MeshFilter>()) {
        entity->addComponent<MeshFilter>();
    }
    if (!entity->hasComponent<MeshRender>()) {
        entity->addComponent<MeshRender>();
    }
    BuildMesh();
}

void ChunkController::BuildMesh() {
    auto& transform = entity->getComponent<Transform>(); // chunck

    for (int x = 0; x < VoxelSettings::ChunkSize; x++) {
        for (int y = 0; y < VoxelSettings::ChunkSize; y++) {
            for (int z = 0; z < VoxelSettings::ChunkSize; z++) {
                m_blocks[x][y][z].isSolid = false;
            }
        }
    }

    for (int x = 0; x < VoxelSettings::ChunkSize; x++) {
        for (int z = 0; z < VoxelSettings::ChunkSize; z++) {
            auto height = Noise.noise((x + transform.position.x) * 0.005f, (z + transform.position.z) * 0.005f);
            height = (height * 0.5f + 0.5f) * float(VoxelSettings::ChunkSize);

            height = std::floor(height);
            if (height <= 0) {
                height = 1;
            }

            for (int y = 0; y < height; y++) {
                m_blocks[x][y][z].isSolid = true;
                m_blocks[x][y][z].transform.position = Vector3((float)x, (float)y, (float)z);
                m_blocks[x][y][z].transform.scale = {0.5f, 0.5f, 0.5f};
            }
        }
    }

    auto& meshFilter = entity->getComponent<MeshFilter>();
    auto& mesh = meshFilter.getMesh();
    mesh.clear();

    for (int x = 0; x < VoxelSettings::ChunkSize; x++) {
        for (int y = 0; y < VoxelSettings::ChunkSize; y++) {
            for (int z = 0; z < VoxelSettings::ChunkSize; z++) {
                auto& block = m_blocks[x][y][z];

                // verifica se o bloco atual existe
                if (block.isSolid) {
                    auto& model = block.transform.getglobalmatrix();

                    // Right face
                    if (checkFace({float(x + 1), float(y), float(z)})) {
                        mesh.addVertice(Vector3(model * Vector4(1.0f, -1.0f,  1.0f, 1.0f)), Vector3(0.0f, 0.0f, 1.0f));
                        mesh.addVertice(Vector3(model * Vector4(1.0f, -1.0f, -1.0f, 1.0f)), Vector3(0.0f, 0.0f, 1.0f));
                        mesh.addVertice(Vector3(model * Vector4(1.0f,  1.0f, -1.0f, 1.0f)), Vector3(0.0f, 0.0f, 1.0f));
                        mesh.addVertice(Vector3(model * Vector4(1.0f,  1.0f,  1.0f, 1.0f)), Vector3(0.0f, 0.0f, 1.0f));

                        u32 baseIndex = mesh.getVerticeCount() - 4;
                        mesh.addtriangle(baseIndex, baseIndex + 1, baseIndex + 2);
                        mesh.addtriangle(baseIndex, baseIndex + 2, baseIndex + 3);
                    }

                    // Left face
                    if (checkFace({float(x - 1), float(y), float(z)})) {
                        mesh.addVertice(Vector3(model * Vector4(-1.0f, -1.0f,  1.0f, 1.0f)), Vector3(0.0f, 0.0f, -1.0f));
                        mesh.addVertice(Vector3(model * Vector4(-1.0f,  1.0f,  1.0f, 1.0f)), Vector3(0.0f, 0.0f, -1.0f));
                        mesh.addVertice(Vector3(model * Vector4(-1.0f,  1.0f, -1.0f, 1.0f)), Vector3(0.0f, 0.0f, -1.0f));
                        mesh.addVertice(Vector3(model * Vector4(-1.0f, -1.0f, -1.0f, 1.0f)), Vector3(0.0f, 0.0f, -1.0f));

                        u32 baseIndex = mesh.getVerticeCount() - 4;
                        mesh.addtriangle(baseIndex, baseIndex + 1, baseIndex + 2);
                        mesh.addtriangle(baseIndex, baseIndex + 2, baseIndex + 3);
                    }
                    
                    // Top face
                    if (checkFace({float(x), float(y + 1), float(z)})) {
                        mesh.addVertice(Vector3(model * Vector4(-1.0f, 1.0f,  1.0f, 1.0f)), Vector3(0.0f, 1.0f, 0.0f));
                        mesh.addVertice(Vector3(model * Vector4( 1.0f, 1.0f,  1.0f, 1.0f)), Vector3(0.0f, 1.0f, 0.0f));
                        mesh.addVertice(Vector3(model * Vector4( 1.0f, 1.0f, -1.0f, 1.0f)), Vector3(0.0f, 1.0f, 0.0f));
                        mesh.addVertice(Vector3(model * Vector4(-1.0f, 1.0f, -1.0f, 1.0f)), Vector3(0.0f, 1.0f, 0.0f));

                        u32 baseIndex = mesh.getVerticeCount() - 4;
                        mesh.addtriangle(baseIndex, baseIndex + 1, baseIndex + 2);
                        mesh.addtriangle(baseIndex, baseIndex + 2, baseIndex + 3);
                    }

                    // Bottom face
                    if (checkFace({float(x), float(y - 1), float(z)})) {
                        mesh.addVertice(Vector3(model * Vector4(-1.0f, -1.0f,  1.0f, 1.0f)), Vector3(0.0f, -1.0f, 0.0f));
                        mesh.addVertice(Vector3(model * Vector4(-1.0f, -1.0f, -1.0f, 1.0f)), Vector3(0.0f, -1.0f, 0.0f));
                        mesh.addVertice(Vector3(model * Vector4( 1.0f, -1.0f, -1.0f, 1.0f)), Vector3(0.0f, -1.0f, 0.0f));
                        mesh.addVertice(Vector3(model * Vector4( 1.0f, -1.0f,  1.0f, 1.0f)), Vector3(0.0f, -1.0f, 0.0f));

                        u32 baseIndex = mesh.getVerticeCount() - 4;
                        mesh.addtriangle(baseIndex, baseIndex + 1, baseIndex + 2);
                        mesh.addtriangle(baseIndex, baseIndex + 2, baseIndex + 3);
                    }

                    // Front face
                    if (checkFace({float(x), float(y), float(z + 1)})) {
                        mesh.addVertice(Vector3(model * Vector4(-1.0f, -1.0f, 1.0f, 1.0f)), Vector3(1.0f, 0.0f, 0.0f));
                        mesh.addVertice(Vector3(model * Vector4( 1.0f, -1.0f, 1.0f, 1.0f)), Vector3(1.0f, 0.0f, 0.0f));
                        mesh.addVertice(Vector3(model * Vector4( 1.0f,  1.0f, 1.0f, 1.0f)), Vector3(1.0f, 0.0f, 0.0f));
                        mesh.addVertice(Vector3(model * Vector4(-1.0f,  1.0f, 1.0f, 1.0f)), Vector3(1.0f, 0.0f, 0.0f));

                        u32 baseIndex = mesh.getVerticeCount() - 4;
                        mesh.addtriangle(baseIndex, baseIndex + 1, baseIndex + 2);
                        mesh.addtriangle(baseIndex, baseIndex + 2, baseIndex + 3);
                    }

                    // Back face
                    if (checkFace({float(x), float(y), float(z - 1)})) {
                        mesh.addVertice(Vector3(model * Vector4(-1.0f, -1.0f, -1.0f, 1.0f)), Vector3(-1.0f, 0.0f, 0.0f));
                        mesh.addVertice(Vector3(model * Vector4(-1.0f,  1.0f, -1.0f, 1.0f)), Vector3(-1.0f, 0.0f, 0.0f));
                        mesh.addVertice(Vector3(model * Vector4( 1.0f,  1.0f, -1.0f, 1.0f)), Vector3(-1.0f, 0.0f, 0.0f));
                        mesh.addVertice(Vector3(model * Vector4( 1.0f, -1.0f, -1.0f, 1.0f)), Vector3(-1.0f, 0.0f, 0.0f));

                        u32 baseIndex = mesh.getVerticeCount() - 4;
                        mesh.addtriangle(baseIndex, baseIndex + 1, baseIndex + 2);
                        mesh.addtriangle(baseIndex, baseIndex + 2, baseIndex + 3);
                    }
                }
            }
        }
    }

    auto& meshRender = entity->getComponent<MeshRender>();

    auto& material = meshRender.addMaterial("default");
    material.albedo = {1.0f, 0.0f, 1.0f, 1.0f};
    material.ao = 1.0f;
    material.roughness = 0.32f;
    material.metallic = 0.2f;

    meshRender.buildMesh();
}

const bool ChunkController::checkFace(const Vector3 &position) {
    int x = static_cast<int>(position.x);
    int y = static_cast<int>(position.y);
    int z = static_cast<int>(position.z);

    if (x < 0 || x >= VoxelSettings::ChunkSize ||
        y < 0 || y >= VoxelSettings::ChunkSize ||
        z < 0 || z >= VoxelSettings::ChunkSize ||
        !m_blocks[x][y][z].isSolid
    ) {
        return true;
    }

    return false;
}

