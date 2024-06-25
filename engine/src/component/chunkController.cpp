#include "chunkController.h"

#include "meshFilter.h"
#include "MeshRender.h"
#include "Transform.h"

SimplexNoise ChunkController::Noise;

int ChunkController::ChunkSize = 16;

ChunkController::ChunkController() {
    face[FaceRender_Top] = true;
    face[FaceRender_Bottom] = true;
    face[FaceRender_Front] = true;
    face[FaceRender_Back] = true;
    face[FaceRender_Right] = true;
    face[FaceRender_Left] = true;
}

void ChunkController::OnInit() {
    if (!entity->hasComponent<MeshFilter>()) {
        entity->addComponent<MeshFilter>();
    }
    if (!entity->hasComponent<MeshRender>()) {
        entity->addComponent<MeshRender>();
    }
    BuildMesh();
}

/*
// Front face
glColor3f(1.0f, 0.0f, 0.0f); // Vermelho
glVertex3f(-0.5f, -0.5f, 0.5f); // Canto inferior esquerdo
glVertex3f(0.5f, -0.5f, 0.5f); // Canto inferior direito
glVertex3f(0.5f, 0.5f, 0.5f); // Canto superior direito
glVertex3f(-0.5f, 0.5f, 0.5f); // Canto superior esquerdo

// Back face
glColor3f(0.0f, 0.0f, 1.0f); // Azul
glVertex3f(-0.5f, -0.5f, -0.5f); // Canto inferior esquerdo
glVertex3f(-0.5f, 0.5f, -0.5f); // Canto superior esquerdo
glVertex3f(0.5f, 0.5f, -0.5f); // Canto superior direito
glVertex3f(0.5f, -0.5f, -0.5f); // Canto inferior direito

// Left face
glColor3f(0.0f, 1.0f, 0.0f); // Verde
glVertex3f(-0.5f, -0.5f, 0.5f); // Canto inferior frontal
glVertex3f(-0.5f, 0.5f, 0.5f); // Canto superior frontal
glVertex3f(-0.5f, 0.5f, -0.5f); // Canto superior traseiro
glVertex3f(-0.5f, -0.5f, -0.5f); // Canto inferior traseiro

// Right face
glColor3f(1.0f, 1.0f, 0.0f); // Amarelo
glVertex3f(0.5f, -0.5f, 0.5f); // Canto inferior frontal
glVertex3f(0.5f, -0.5f, -0.5f); // Canto inferior traseiro
glVertex3f(0.5f, 0.5f, -0.5f); // Canto superior traseiro
glVertex3f(0.5f, 0.5f, 0.5f); // Canto superior frontal

// Top face
glColor3f(1.0f, 0.0f, 1.0f); // Magenta
glVertex3f(-0.5f, 0.5f, 0.5f); // Canto esquerdo frontal
glVertex3f(0.5f, 0.5f, 0.5f); // Canto direito frontal
glVertex3f(0.5f, 0.5f, -0.5f); // Canto direito traseiro
glVertex3f(-0.5f, 0.5f, -0.5f); // Canto esquerdo traseiro

// Bottom face
glColor3f(0.5f, 0.5f, 0.5f); // Cinza
glVertex3f(-0.5f, -0.5f, 0.5f); // Canto esquerdo frontal
glVertex3f(-0.5f, -0.5f, -0.5f); // Canto esquerdo traseiro
glVertex3f(0.5f, -0.5f, -0.5f); // Canto direito traseiro
glVertex3f(0.5f, -0.5f, 0.5f); // Canto direito frontal

glEnd(); // Termina de definir os quadriláteros
*/

void ChunkController::BuildMesh() {
    auto& transform = entity->getComponent<Transform>(); // chunck

    for (int x = 0; x < ChunkSize; x++) {
        for (int y = 0; y < ChunkSize; y++) {
            for (int z = 0; z < ChunkSize; z++) {
                m_blocks[x][y][z].isSolid = false;
            }
        }
    }

    for (int x = 0; x < ChunkSize; x++) {
        for (int z = 0; z < ChunkSize; z++) {
            auto height = Noise.noise((x + transform.position.x) * 0.005f, (z + transform.position.z) * 0.005f);
            height = (height * 0.5f + 0.5f) * float(ChunkSize);

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

    for (int x = 0; x < ChunkSize; x++) {
        for (int y = 0; y < ChunkSize; y++) {
            for (int z = 0; z < ChunkSize; z++) {
                auto& block = m_blocks[x][y][z];

                // verifica se o bloco atual existe
                if (block.isSolid) {
                    auto& model = block.transform.getglobalmatrix();

                    // verifica se existe o bloco acima existe se for o caso não adiciona um face.
                    // Vericica se o bloco acima esta na borda da chunck se tiver adiciona uma face

                    // Top face
                    if (face[FaceRender_Top] && checkFace({float(x), float(y + 1), float(z)})) {
                        mesh.addVertice(Vector3(model * Vector4(-1.0f, 1.0f,  1.0f, 1.0f)), Vector3(0.0f, 1.0f, 0.0f));
                        mesh.addVertice(Vector3(model * Vector4( 1.0f, 1.0f,  1.0f, 1.0f)), Vector3(0.0f, 1.0f, 0.0f));
                        mesh.addVertice(Vector3(model * Vector4( 1.0f, 1.0f, -1.0f, 1.0f)), Vector3(0.0f, 1.0f, 0.0f));
                        mesh.addVertice(Vector3(model * Vector4(-1.0f, 1.0f, -1.0f, 1.0f)), Vector3(0.0f, 1.0f, 0.0f));

                        u32 baseIndex = mesh.getVerticeCount() - 4;
                        mesh.addtriangle(baseIndex, baseIndex + 1, baseIndex + 2);
                        mesh.addtriangle(baseIndex, baseIndex + 2, baseIndex + 3);
                    }

                    // Bottom face
                    if (face[FaceRender_Bottom] && checkFace({float(x), float(y - 1), float(z)})) {
                        mesh.addVertice(Vector3(model * Vector4(-1.0f, -1.0f,  1.0f, 1.0f)), Vector3(0.0f, -1.0f, 0.0f));
                        mesh.addVertice(Vector3(model * Vector4(-1.0f, -1.0f, -1.0f, 1.0f)), Vector3(0.0f, -1.0f, 0.0f));
                        mesh.addVertice(Vector3(model * Vector4( 1.0f, -1.0f, -1.0f, 1.0f)), Vector3(0.0f, -1.0f, 0.0f));
                        mesh.addVertice(Vector3(model * Vector4( 1.0f, -1.0f,  1.0f, 1.0f)), Vector3(0.0f, -1.0f, 0.0f));

                        u32 baseIndex = mesh.getVerticeCount() - 4;
                        mesh.addtriangle(baseIndex, baseIndex + 1, baseIndex + 2);
                        mesh.addtriangle(baseIndex, baseIndex + 2, baseIndex + 3);
                    }

                    // Front face
                    if (false && face[FaceRender_Front] && checkFace({float(x), float(y), float(z + 1)}) && (z + 1 != 0 && z + 1 != ChunkSize)) {
                        mesh.addVertice(Vector3(model * Vector4(-1.0f, -1.0f, 1.0f, 1.0f)), Vector3(1.0f, 0.0f, 0.0f));
                        mesh.addVertice(Vector3(model * Vector4( 1.0f, -1.0f, 1.0f, 1.0f)), Vector3(1.0f, 0.0f, 0.0f));
                        mesh.addVertice(Vector3(model * Vector4( 1.0f,  1.0f, 1.0f, 1.0f)), Vector3(1.0f, 0.0f, 0.0f));
                        mesh.addVertice(Vector3(model * Vector4(-1.0f,  1.0f, 1.0f, 1.0f)), Vector3(1.0f, 0.0f, 0.0f));

                        u32 baseIndex = mesh.getVerticeCount() - 4;
                        mesh.addtriangle(baseIndex, baseIndex + 1, baseIndex + 2);
                        mesh.addtriangle(baseIndex, baseIndex + 2, baseIndex + 3);
                    }

                    // Back face
                    if (face[FaceRender_Back] && checkFace({float(x), float(y), float(z - 1)}) && (z - 1 != 0 && z - 1 != ChunkSize)) {
                        mesh.addVertice(Vector3(model * Vector4(-1.0f, -1.0f, -1.0f, 1.0f)), Vector3(-1.0f, 0.0f, 0.0f));
                        mesh.addVertice(Vector3(model * Vector4(-1.0f,  1.0f, -1.0f, 1.0f)), Vector3(-1.0f, 0.0f, 0.0f));
                        mesh.addVertice(Vector3(model * Vector4( 1.0f,  1.0f, -1.0f, 1.0f)), Vector3(-1.0f, 0.0f, 0.0f));
                        mesh.addVertice(Vector3(model * Vector4( 1.0f, -1.0f, -1.0f, 1.0f)), Vector3(-1.0f, 0.0f, 0.0f));

                        u32 baseIndex = mesh.getVerticeCount() - 4;
                        mesh.addtriangle(baseIndex, baseIndex + 1, baseIndex + 2);
                        mesh.addtriangle(baseIndex, baseIndex + 2, baseIndex + 3);
                    }

                    /*
                    // Right face
                    if (face[FaceRender_Right] && checkFace({float(x), float(y), float(z + 1)})) {
                        mesh.addVertice(Vector3(model * Vector4( 1.0f, -1.0f, 1.0f, 1.0f)), Vector3(0.0f, 0.0f, 1.0f));
                        mesh.addVertice(Vector3(model * Vector4( 1.0f,  1.0f, 1.0f, 1.0f)), Vector3(0.0f, 0.0f, 1.0f));
                        mesh.addVertice(Vector3(model * Vector4(-1.0f,  1.0f, 1.0f, 1.0f)), Vector3(0.0f, 0.0f, 1.0f));
                        mesh.addVertice(Vector3(model * Vector4(-1.0f, -1.0f, 1.0f, 1.0f)), Vector3(0.0f, 0.0f, 1.0f));

                        u32 baseIndex = mesh.getVerticeCount() - 4;
                        mesh.addtriangle(baseIndex, baseIndex + 1, baseIndex + 2);
                        mesh.addtriangle(baseIndex, baseIndex + 2, baseIndex + 3);
                    }

                    // Left face
                    if (face[FaceRender_Left] && checkFace({float(x), float(y), float(z - 1)})) {
                        mesh.addVertice(Vector3(model * Vector4( 1.0f, -1.0f, -1.0f, 1.0f)), Vector3(0.0f, 0.0f, -1.0f));
                        mesh.addVertice(Vector3(model * Vector4( 1.0f,  1.0f, -1.0f, 1.0f)), Vector3(0.0f, 0.0f, -1.0f));
                        mesh.addVertice(Vector3(model * Vector4(-1.0f,  1.0f, -1.0f, 1.0f)), Vector3(0.0f, 0.0f, -1.0f));
                        mesh.addVertice(Vector3(model * Vector4(-1.0f, -1.0f, -1.0f, 1.0f)), Vector3(0.0f, 0.0f, -1.0f));

                        u32 baseIndex = mesh.getVerticeCount() - 4;
                        mesh.addtriangle(baseIndex, baseIndex + 2, baseIndex + 1);
                        mesh.addtriangle(baseIndex, baseIndex + 3, baseIndex + 2);
                    }
                    */
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
    if (((static_cast<int>(position.x) < 0 || static_cast<int>(position.x) >= ChunkSize) || (static_cast<int>(position.y) < 0 || static_cast<int>(position.y) >= ChunkSize) || (static_cast<int>(position.z) < 0 || static_cast<int>(position.z) >= ChunkSize)) || !m_blocks[static_cast<int>(position.x)][static_cast<int>(position.y)][static_cast<int>(position.z)].isSolid) {
        return true;
    }
    return false;
}
