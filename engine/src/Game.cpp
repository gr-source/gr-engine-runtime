#include "Game.h"

#include "timer.h"
#include "screen.h"
#include "memory.h"
#include "skybox.h"
#include "settings.h"
#include "renderer.h"
#include "console.hpp"
#include "screenRender.hpp"
#include "renderManager.h"
#include "editorController.h"
#include "postProcessing.hpp"

#include "core/filmbox/filmbox.hpp"
#include "core/scene.hpp"

#include "framebuffer/shadowbuffer.h"
#include "framebuffer/screenbuffer.h"
#include "framebuffer/imguibuffer.h"

#include "resources/resources.h"

#include "component/meshFilter.hpp"
#include "component/meshRender.hpp"
#include "component/transform.hpp"
#include "component/animator.hpp"
#include "component/camera.hpp"
#include "component/light.hpp"

#include "guizmo/ImGuizmo.h"

#include "ui/ImGuiRender.h"
#include "ui/View.h"

#include "filesystem/filesystem.h"
#include "filesystem/stream.h"

#include "scriptManager/scriptManager.hpp"

#include <filesystem>
#include <sstream>
#include <iomanip>

#define IMGUI_PAYLOAD_GAMEOBJECT "##payload_game_object"

const std::string getfilesize(std::size_t sz) {
    float kb = sz / 1024.0f;
    float mb = kb / 1024.0f;
    float gb = mb / 1024.0f;

    std::ostringstream oss;

    if (gb >= 1.0f) {
        oss << std::fixed << std::setprecision(2) << gb << "GB";
    } else if (mb >= 1.0f) {
        oss << std::fixed << std::setprecision(2) << mb << "MB";
    } else if (kb >= 1.0f) {
        oss << std::fixed << std::setprecision(2) << kb << "KB";
    } else {
        oss << std::fixed << std::setprecision(2) << sz << "B";
    }
    return oss.str();
}

enum IconType {
    ICON_FOLDER       = 0,
    ICON_FILE         = 1,
    ICON_BACK_FILE    = 2,
    ICON_GAMEOBJECT   = 3,
	ICON_PENCIL       = 4,
	ICON_LIGHT        = 5,
	ICON_TEXTURE      = 6,
	ICON_PNG          = 7,
	ICON_PLAY_GAME    = 8,
	ICON_STOP_GAME    = 9,
	ICON_BIN          = 10,
	ICON_IMPORT       = 11,
	ICON_ASCENDENTE   = 12,
	ICON_DESCENDENTE  = 13,
	ICON_EDIT         = 14
};

std::unordered_map<IconType, grr::gTexture *> iconsUI;

std::vector<std::unique_ptr<Filmbox>> m_filmboxList;

/* Seleciona um objeto no scenes(UI)*/
GameObject *Game::s_objectSelected = nullptr;

/* Textura do play button */
grr::gTexture *Game::s_texturePlayGame = nullptr;

ImGuiIO* Game::s_io = nullptr;

bool Game::bScriptReloaded = false;

bool Game::bMenuAdd = false;

ImVec4 Game::FPSColor = {255, 255, 255, 255};

ImGuizmo::OPERATION Game::m_guizmoOperation = ImGuizmo::TRANSLATE;

ImGuizmo::MODE Game::m_guizmoManipulate = ImGuizmo::WORLD;

u32 Game::s_maxProgress = 0;

void Game::Initialize() {
    s_io = &ImGui::GetIO();

    loadReources();

    Settings::IsGameRunning = false;
}

// ===================================== WARNING =========================================================
#include "core/binary/gHeader.hpp"


void WriteNode(const FbxScene *scene, const FbxNode &node) {
    Console::Print(LogInfo::INFO, "Writing model...");

    auto path = Filesystem::GetFile("/model/" + node.getName() + ".gmd");

    std::ofstream os(path, std::ios_base::binary);
    if (!os) {
        std::cerr << "Failed to open file for writing." << std::endl;
        return;
    }

    gHeader pHeader;
    pHeader.signature[0] = 'g';
    pHeader.signature[1] = 'r';
    pHeader.signature[2] = 'm';
    pHeader.signature[3] = 'd';
    pHeader.version = 1.0f;

    if (!os.write(reinterpret_cast<const char *>(&pHeader), sizeof(gHeader))) {
        std::cerr << "Failed to write header to file." << std::endl;
        return;
    }

    gModel pModel;
    std::strcpy(pModel.m_name, node.getName().c_str());

    pModel.m_scale = node.getScale();
    pModel.m_rotation = node.getRotation();
    pModel.m_position = node.getPosition();

    if (!os.write(reinterpret_cast<const char *>(&pModel), sizeof(gModel))) {
        return;
    }

    auto &meshIndex = node.getMeshes();

    // escreve a quantidade de mesh
    grm::u32 meshCount = static_cast<grm::u32>(meshIndex.size());
    if (!os.write(reinterpret_cast<const char *>(&meshCount), sizeof(grm::u32))) {
        std::cerr << "Failed to write mesh count to file." << std::endl;
        return;
    }

    // alloca as meshes na memória em forma de pilha (fácil acesso)
    std::vector<gMesh> meshes(meshCount);

    for (grm::u32 i = 0; i < meshCount; ++i) {
        auto &fbxMesh = scene->getMesh(meshIndex[i]);

        auto &vertices = fbxMesh.getVertices();
        auto &indices = fbxMesh.getIndices();

        auto &pMesh = meshes[i];
        std::strcpy(pMesh.m_name, fbxMesh.getName().c_str());
        
        pMesh.m_verticeCount = static_cast<grm::u32>(vertices.size());
        pMesh.m_vertices = new gVertex[pMesh.m_verticeCount];
        for (auto i=0;i<pMesh.m_verticeCount;++i) {
            auto &pVertice = pMesh.m_vertices[i];

            auto &fbxVertice = vertices[i];

            pVertice.position = fbxVertice.position;
            pVertice.normal = fbxVertice.normal;
            pVertice.texCoord = fbxVertice.texCoord;
        }

        pMesh.m_indicesCount = static_cast<grm::u32>(indices.size());
        pMesh.m_indices = new grm::u32[pMesh.m_indicesCount];
        for (int i=0;i<pMesh.m_indicesCount;++i) {
            pMesh.m_indices[i] = indices[i];
        }
    }

    // Escrever os dados das meshes no arquivo
    for (const auto &mesh : meshes) {
        if (!os.write(reinterpret_cast<const char *>(mesh.m_name), sizeof(char) * 256) ||
            !os.write(reinterpret_cast<const char *>(&mesh.m_verticeCount), sizeof(grm::u32)) ||
            !os.write(reinterpret_cast<const char *>(mesh.m_vertices), mesh.m_verticeCount * sizeof(gVertex)) ||
            !os.write(reinterpret_cast<const char *>(&mesh.m_indicesCount), sizeof(grm::u32)) ||
            !os.write(reinterpret_cast<const char *>(mesh.m_indices), mesh.m_indicesCount * sizeof(grm::u32))) {
            std::cerr << "Failed to write mesh data to file." << std::endl;
            return;
        }
    }

    // Liberar a memória alocada
    for (auto &mesh : meshes) {
        delete[] mesh.m_vertices;
        delete[] mesh.m_indices;
    }

    os.close();
}

void PostProcess(const FbxScene *scene, Filmbox *filmbox) {
    auto& fRootNode = scene->getRootNode();
    for (const auto& node : fRootNode.getChildren()) {
        WriteNode(scene, node);
    }
    delete filmbox;
}

void Game::DropFile(const std::filesystem::path &path) {
    if (path.extension() != ".fbx") {
        return;
    }
    Console::Print(LogInfo::INFO, "Loading fbx: " + path.extension().string());

    auto *filmbox = new Filmbox(path.string());
    filmbox->setPostProcess(PostProcess);
    filmbox->load();
}

// ===================================== END =========================================================


void Game::RenderUI() {
    if (Resources::m_processCount != 0) {
        ProgressLoad();
        return;
    }
    
    RenderDocking();

    RenderScenesUI();

    RenderViewportUI();

    RenderConsoleUI();

    RenderPropertiUI();
}

void Game::RenderDocking() {
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("GR - GameEngine 3D", nullptr, ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoTitleBar);
    if (s_io->ConfigFlags & ImGuiConfigFlags_DockingEnable) {
        ImGui::DockSpace(ImGui::GetID("##RenderDocking"), ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);
    }
    ImGui::End();
    ImGui::PopStyleVar();
}

void Game::RenderScenesUI() {
    ImGui::Begin("Cenario", nullptr, ImGuiWindowFlags_NoResize);
    ImGui::SetNextItemOpen(true, ImGuiCond_Appearing);
    if (ImGui::TreeNodeEx("default", ImGuiTreeNodeFlags_SpanFullWidth)) {
        if (ImGui::BeginDragDropTarget()) {
            ImGuiDragDropFlags target_flags = 0;
            // target_flags |= ImGuiDragDropFlags_AcceptBeforeDelivery;
            // target_flags |= ImGuiDragDropFlags_SourceAllowNullID;

            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(IMGUI_PAYLOAD_GAMEOBJECT, target_flags)) {
                std::string name((char*)payload->Data, payload->DataSize);
                if (!name.empty()) {
                    auto target = GameObject::Find(name);

                    auto &ts = target->getTransform();
                    ts.setParent(nullptr);
                    
                    Scene::Add(target);
                }
            }
            ImGui::EndDragDropTarget();
        }
        
        if (Scene::GetGameObject) {
            for (auto& gameObject : Scene::GetGameObject()) {
                auto& transform = gameObject->getTransform();

                ReadHierarchy(&transform);
            }
        }
        
        ImGui::TreePop();
    }

    if (s_io->KeyCtrl && ImGui::IsKeyReleased(ImGuiKey_A)) {
        bMenuAdd = !bMenuAdd;
    }
    
    ImGui::End();

    if (bMenuAdd) {
        MenuAdd();
    }
}

void Game::ReadHierarchy(Transform *transform) {
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Selected | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanFullWidth;

    auto gameObject = transform->getGameObject();
    auto name = gameObject->getName();

    if (s_objectSelected != gameObject) {
        flags &= ~ImGuiTreeNodeFlags_Selected;
    }

    if (transform->hasChildren()) {
        flags &= ~ImGuiTreeNodeFlags_Leaf;
    }

    const bool isOpen = ImGui::TreeNodeEx(name.c_str(), flags);

    if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
        s_objectSelected = gameObject;
    }

    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceNoDisableHover | ImGuiDragDropFlags_SourceNoHoldToOpenOthers)) {
        ImGui::Text("Moving \"%s\"", name.c_str());
        ImGui::SetDragDropPayload(IMGUI_PAYLOAD_GAMEOBJECT, name.data(), name.size());
        ImGui::EndDragDropSource();
    }

    if (ImGui::BeginDragDropTarget()) {
        ImGuiDragDropFlags target_flags = 0;
        // target_flags |= ImGuiDragDropFlags_AcceptBeforeDelivery;
        // target_flags |= ImGuiDragDropFlags_SourceAllowNullID;

        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(IMGUI_PAYLOAD_GAMEOBJECT, target_flags)) {
            std::string name((char*)payload->Data, payload->DataSize);
            if (!name.empty()) {
                auto target = GameObject::Find(name);

                auto &ts = target->getTransform();
                ts.setParent(transform);
                
                Scene::Remove(target);
            }
        }
        ImGui::EndDragDropTarget();
    }

    if (isOpen) {
        for (auto& child : transform->getChildren()) {
            ReadHierarchy(child);
        }
        ImGui::TreePop();
    }
}

void Game::RenderViewportUI() {
    static Imguibuffer& m_imguibuffer = Imguibuffer::getInstance();

    ImGui::Begin("Viewport", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

    ImGuizmo::SetDrawlist();

    ImVec2 size = ImGui::GetWindowSize();

    ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, size.x, size.y);

    static ImVec2 lastSize = ImGui::GetContentRegionAvail();
    ImVec2 curSize = ImGui::GetContentRegionAvail();

    if (lastSize.x != curSize.x || lastSize.y != curSize.y && curSize.x > 5.0f && curSize.y > 5.0f || bScriptReloaded) {
        auto &viewport = Settings::ViewportSize = {static_cast<int>(curSize.x), static_cast<int>(curSize.y)};

        auto camera = Camera::Current();
        if (camera) {
            camera->setAspectRatio(static_cast<float>(viewport.x) / static_cast<float>(viewport.y));
        }

        PostProcessing::Resize(viewport);
        
        lastSize = curSize;
        bScriptReloaded = false;
    }

    ImGui::Image(reinterpret_cast<ImTextureID>(m_imguibuffer.gettexture()), curSize, ImVec2(0, 1), ImVec2(1, 0));

    // ImGui::SetCursorPos({curSize.x / 2.0f, curSize.y / 2.0f});
// 
    // ImGui::Text("X");

    if (s_objectSelected != nullptr) {
        if (s_io->KeyCtrl && ImGui::IsKeyReleased(ImGuiKey_T)) {
            m_guizmoOperation = ImGuizmo::TRANSLATE;
        }
        if (s_io->KeyCtrl && ImGui::IsKeyReleased(ImGuiKey_R)) {
            m_guizmoOperation = ImGuizmo::ROTATE;
        }
        if (s_io->KeyCtrl && ImGui::IsKeyReleased(ImGuiKey_S)) {
            m_guizmoOperation = ImGuizmo::SCALE;
        }
        if (s_io->KeyCtrl && ImGui::IsKeyReleased(ImGuiKey_Space)) {
            if (m_guizmoManipulate == ImGuizmo::WORLD) {
                m_guizmoManipulate = ImGuizmo::LOCAL;
            } else if (m_guizmoManipulate == ImGuizmo::LOCAL) {
                m_guizmoManipulate = ImGuizmo::WORLD;
            }
        }
        
        Transform& transform = s_objectSelected->getTransform();

        Matrix4x4 model = transform.getGlobalMatrix();

        auto camera = Camera::Current();

        if (ImGuizmo::Manipulate(camera->getViewmatrix().data, camera->getPerspesctive().data, m_guizmoOperation, m_guizmoManipulate, model.data)) {
            model.decompose(transform.scale, transform.rotation, transform.position);
        }
    }

    const ImVec2 wSize =  ImGui::GetCursorPos();

    // Play button
    {
        ImGui::SetCursorPos(ImVec2(curSize.x / 2 - wSize.x, 32));

        if (ImGui::ImageButton(reinterpret_cast<ImTextureID>(s_texturePlayGame->getID()), ImVec2(16, 16))) {
            Settings::IsGameRunning =! Settings::IsGameRunning;

            if (Settings::IsGameRunning) {
                s_texturePlayGame = iconsUI[IconType::ICON_STOP_GAME];

                StopGame();

                s_objectSelected = nullptr;
            } else {
                s_texturePlayGame = iconsUI[IconType::ICON_PLAY_GAME];

                PlayGame();

                bScriptReloaded = true;
            }
        }
    }

    if (Time::FPS > 50) {
        FPSColor = {0, 255, 0, 255};
    } else if (Time::FPS > 25) {
        FPSColor = {255, 255, 0, 255};
    } else if (Time::FPS > 0) {
        FPSColor = {255, 0, 0, 255};
    }

    // Fps Draw
    {
        std::string fps = "FPS: " + std::to_string(Time::FPS);

        auto textSize = ImGui::CalcTextSize(fps.c_str());

        auto curtorPos = ImVec2(curSize.x - textSize.x, textSize.y * 2.5f);

        ImGui::SetCursorPos(curtorPos);

        ImGui::TextColored(FPSColor, "%s", fps.c_str());
    }

    ImGui::SetCursorPos({5.0f, 35.0f});

    ImGui::PushStyleColor(ImGuiCol_Text, {0.0f, 0.0f, 1.0f, 1.0f});

    ImGui::BeginGroup();
    if (ImGui::RadioButton("Local", m_guizmoManipulate == ImGuizmo::LOCAL)) {
        m_guizmoManipulate = ImGuizmo::LOCAL;
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("World", m_guizmoManipulate == ImGuizmo::WORLD)) {
        m_guizmoManipulate = ImGuizmo::WORLD;
    }
    ImGui::EndGroup();

    ImGui::PopStyleColor();

    ImGui::End();
}

void Game::RenderFilesUI() {
    static std::filesystem::path root = Filesystem::GetFile("");
    ImGui::Columns(5, "#Collums1", false);

    grr::gTexture *gtCurrentTexture = iconsUI[ICON_BACK_FILE];

    if (ImGui::ImageButton(reinterpret_cast<ImTextureID>(gtCurrentTexture->getID()), ImVec2(32, 32), ImVec2(0, 1), ImVec2(1, 0)) && root.has_parent_path()) {
        root = root.parent_path();
    }

    for (auto& file : std::filesystem::directory_iterator(root)) {
        ImGui::NextColumn();

        auto &path = file.path();

        ImGui::PushID(path.filename().c_str());

        gtCurrentTexture = iconsUI[ICON_FILE];
        if (file.is_directory()) {
            gtCurrentTexture = iconsUI[ICON_FOLDER];
        }

        if (ImGui::ImageButton(reinterpret_cast<ImTextureID>(gtCurrentTexture->getID()), ImVec2(32, 32), ImVec2(0, 1), ImVec2(1, 0))) {
            if (file.is_directory()) {
                root = path;
            }
        }

        ImGui::Text("%s", path.filename().c_str());

        ImGui::PopID();
    }
    ImGui::Columns(1);
}

void Game::RenderConsoleUI() {
    static NavegationFlags flags = NavegationFlags_console;

    ImGui::Begin("Navegador", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_MenuBar);
    if (ImGui::BeginMenuBar()) {
        if (ImGui::MenuItem("Console")) {
            flags = NavegationFlags_console;
        }
        if (ImGui::MenuItem("Files")) {
            flags = NavegationFlags_files;
        }
        if (ImGui::MenuItem("Models")) {
            flags = NavegationFlags_models;
        }
        ImGui::EndMenuBar();
    }

    if ((flags & NavegationFlags_files) == NavegationFlags_files) {
        RenderFilesUI();
    }

    if ((flags & NavegationFlags_models) == NavegationFlags_models) {
    }

    if ((flags & NavegationFlags_console) == NavegationFlags_console) {
        auto logs = Console::GetListLogs();

        for (const auto& log : logs) {
            ImVec4 color = {1.0f, 1.0f, 1.0f, 1.0f};
            std::string type = "[ DEFAULT ]";

            switch (log.type) {
            case LogInfo::LogType::INFO:
                color = {0.0f, 1.0f, 0.0f, 1.0f};
                type = "[ INFO ]";
                break;
            case LogInfo::LogType::WARNING:
                color = {1.0f, 1.0f, 0.0f, 1.0f};
                type = "[ WARNING ]";
                break;
            case LogInfo::LogType::ERROR:
                color = {1.0f, 0.0f, 0.0f, 1.0f};
                type = "[ ERROR ]";
                break;
            default:
                break;
            }

            ImGui::TextColored(color, "%s", type.c_str());
            ImGui::SameLine();
            ImGui::Text("%s", log.message.c_str());
        }
        if (Console::bChanged) {
            ImGui::SetScrollHereY(1.0f);
            Console::bChanged = false;
        }
    }

    ImGui::End();
}

void Game::RenderPropertiUI() {
    ImGui::Begin("Propriedades", nullptr, ImGuiWindowFlags_NoResize);

    if (ImGui::BeginTabBar("##propertimenubar")) {
        if (s_objectSelected && ImGui::BeginTabItem("GameObject", nullptr)) {
            ImGui::Image(reinterpret_cast<ImTextureID>(iconsUI[IconType::ICON_GAMEOBJECT]->getID()), ImVec2(16, 16));

            ImGui::SameLine();

            ImGui::Text("[ %s - %s ]", s_objectSelected->getName().c_str(), s_objectSelected->getTag().c_str());

            /*
            auto entity = objectselected->getEntity();

            for (auto&& [name, componenet] : entity->getComponents()) {
                ImGui::PushID(name.c_str());
                auto isOpen = ImGui::TreeNode(name.c_str());

                if (isOpen) {
                    ImGui::TreePop();
                }

                ImGui::SameLine();

                if (ImGui::Button("X")) {
                    EventManager::addEvent(&Entity::RemoveComponent, entity, name);
                }
                ImGui::PopID();
            }
            */

            s_objectSelected->RenderProperty<Transform>(&Game::OnPropertyTransform);
            s_objectSelected->RenderProperty<MeshRender>(&Game::OnPropertyMeshRender);
            s_objectSelected->RenderProperty<Light>(&Game::OnPropertyLightComponent);
            s_objectSelected->RenderProperty<Animator>(&Game::OnPropertyAnimator);
            /*
            objectselected->RenderProperty<CameraComponent>(&Game::OnPropertyCameraComponent, this);
            objectselected->RenderProperty<ChunkController>(&Game::OnPropertyChunkController, this);
            */

            ImGui::EndTabItem();
        }
        
        ImGui::SetNextItemOpen(true, ImGuiCond_FirstUseEver);
        if (ImGui::BeginTabItem("Estatística", nullptr)) {
            ImGui::SeparatorText("Scripts");
            
            if (ScriptManager::GetEntityCount)
                ImGui::Text("Entidades: %ld", ScriptManager::GetEntityCount());
            ImGui::Text("Memoria: %s", getfilesize(Memory::totalMemoryUsage).c_str());

            ImGui::SeparatorText("Cena");
            ImGui::Text("Vertices: %d | %s", gameStatics.vertices, getfilesize(gameStatics.vertices * sizeof(Vertex3D)).c_str());
            ImGui::Text("Indice: %d | %s", gameStatics.indices, getfilesize(gameStatics.indices * sizeof(u32)).c_str());
            ImGui::Text("Triangulos: %.2f", (gameStatics.indices / 3.0f));

            ImGui::SeparatorText("Resources");
            ImGui::Text("Textures: %ld", Resources::GetTextureCount());
            ImGui::Text("Shaders: %ld", Resources::GetShaderCount());

            ImGui::SeparatorText("Threads");
            ImGui::Text("Update:  %fms  | FPS: %.2f",  gameStatics.updateMs,  (1.0f / gameStatics.updateMs));
            ImGui::Text("Render:  %fms  | FPS: %.2f",  gameStatics.renderMs,  (1.0f / (Time::deltaTime - gameStatics.renderMs)));
            ImGui::Text("Physics: %fms  | FPS: %.2f",  gameStatics.physicsMs, (1.0f / gameStatics.physicsMs));
            ImGui::Text("Script:  %fms  | FPS: %.2f",  gameStatics.scriptMS,  (1.0f / gameStatics.scriptMS));

            ImGui::SeparatorText("Renderização");
            // ImGui::Text("Cameras: %ld",   Camera::Get.size());
            if (ScriptManager::GetListLight)
                ImGui::Text("Luzes: %ld",     ScriptManager::GetListLight().size());
            ImGui::Text("Draw Render: %d/%d", RenderManager::iDrawCount, RenderManager::iDrawRender);
            
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Configuraçoes")) {
            ImGui::Checkbox("Wireframe", &RenderManager::bWireFrame);
            ImGui::Checkbox("Debug Mode", &DebugDraw::bDebugMode);

            ImGui::Text("Camera: ");
            if (Camera::GetCameras) {
                const auto &viewport = Settings::ViewportSize;

                for (auto &camera : Camera::GetCameras()) {
                    auto gameObject = camera->getGameObject();

                    auto name = gameObject->getName();

                    ImGui::PushID(name.c_str());
                    if (ImGui::Button(name.c_str())) {
                        camera->setAspectRatio(static_cast<float>(viewport.x) / static_cast<float>(viewport.y));

                        Camera::SetEnable(camera);
                    }
                    ImGui::PopID();
                }
            }
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
    ImGui::End();
}

/* ********** Propriedades ********** */
void Game::OnPropertyTransform(Transform* transform) {
    if (ImGui::TreeNode("Transform")) {
        const char* text[] {"X", "Y", "Z"};

        ImGui::Text("Posição:");
        ImGui::DragFloatEx("##Posicao", text, transform->position.data, 1.0f, 0.0f, 0.0f, "%.3f");

        ImGui::Text("Escala:");
        ImGui::DragFloatEx("##Escala", text, transform->scale.data, 1.0f, 1.0f, 0.0f, "%.3f");

        Vector3 euler = transform->rotation.eulerAngles();

        ImGui::Text("Rotação:");
        if (ImGui::DragFloatEx("##Rotacao", text, euler.data, 1.0f, -360.0f, 360.0f)) {
            transform->rotation = Math::normalize(Math::euler({Math::rad(euler.x), Math::rad(euler.y), Math::rad(euler.z)}));
        }

        if (ImGui::Button("Save")) {
            std::ofstream is("/home/grsource/Documentos/scn.ct", std::ios_base::ate | std::ios_base::app);
            is << transform->position << " " << euler << std::endl; 
            is.close();
        }
        ImGui::TreePop();
    }
}

void Game::OnPropertyAnimator(Animator *animator) {
    auto &m_current = animator->m_animation;

    if (ImGui::TreeNode("Animator")) {
        auto &armature = animator->m_armature;

        if (armature) {
            ImGui::Text("Armature: %s", armature->getName().c_str());
        } else {
            ImGui::Text("Draging");
        }

        if (ImGui::BeginDragDropTarget()) {
            ImGuiDragDropFlags target_flags = 0;
            target_flags |= ImGuiDragDropFlags_AcceptBeforeDelivery;
            target_flags |= ImGuiDragDropFlags_SourceAllowNullID;

            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(IMGUI_PAYLOAD_GAMEOBJECT, target_flags)) {
                std::string name((char*)payload->Data, payload->DataSize);
                if (!name.empty()) {
                    armature = GameObject::Find(name);
                }
            }
            ImGui::EndDragDropTarget();
        }

        auto isOpen = ImGui::TreeNode("Animations");

        if (isOpen) {
            auto &animations = animator->m_animations;
            for (int i=0;i<animations.size();++i) {
                auto &anim = animations[i];

                auto name = anim.getName();

                ImGui::Selectable(name.c_str(), m_current == &anim);
                if (ImGui::IsItemClicked()) {
                    m_current = &anim;
                }
            }
            ImGui::TreePop();
        }
        ImGui::TreePop();
    }
}

void Game::OnPropertyMeshRender(MeshRender *meshRender) {
    if (ImGui::TreeNode("MeshRender")) {
        for (auto&& [idx, material] : meshRender->getMaterials()) {
            if (ImGui::TreeNode(std::string(material.getName() + "(" + std::to_string(idx) + ")").c_str())) {
                
                auto& albedo = material.getAlbedo();
                {
                    ImGui::SeparatorText("Albedo");
                    auto texture = material.getTexture(0);

                    if (texture != nullptr && texture->isValid()) {
                        ImGui::Image(reinterpret_cast<ImTextureID>(texture->getID()), ImVec2(32, 32));
                        if (ImGui::IsItemHovered() && ImGui::IsKeyDown(ImGuiKey_X)) {
                            texture = nullptr;
                        }
                    } else {

                        if (View::TextureInputUI(ImColor(albedo.x, albedo.y, albedo.z, albedo.w))) {
                            ImGui::OpenPopup("#PopupChangeColorAlbedo");
                        }
                    }

                    /*
                    if (ImGui::BeginDragDropTarget()) {
                        if (const ImGuiPayload* data = ImGui::AcceptDragDropPayload(getDragDrop(DragDrop::TEXTURE), ImGuiDragDropFlags_None)) {
                            std::string path((char*)data->Data, data->DataSize);
                            if (!path.empty()) {
                                material.texture[TexturePBR::ALBEDO] = Resources::Load<Texture>(path);
                            }
                        }
                        ImGui::EndDragDropTarget();
                    }
                    */
                }

                /*
                {
                    ImGui::SeparatorText("Normal");
                    if (material.texture[TexturePBR::NORMAL] != nullptr && material.texture[TexturePBR::NORMAL]->isvalid()) {
                        ImGui::Image(reinterpret_cast<ImTextureID>(material.texture[TexturePBR::NORMAL]->getID()), ImVec2(32, 32));
                        if (ImGui::IsItemHovered() && ImGui::IsKeyDown(ImGuiKey_X)) {
                            material.texture[TexturePBR::NORMAL] = nullptr;
                        }
                    } else {
                        View::TextureInputUI(ImColor(100, 100, 255));
                    }

                    if (ImGui::BeginDragDropTarget()) {
                        if (const ImGuiPayload* data = ImGui::AcceptDragDropPayload(getDragDrop(DragDrop::TEXTURE), ImGuiDragDropFlags_None)) {
                            std::string path((char*)data->Data, data->DataSize);
                            if (!path.empty()) {
                                material.texture[TexturePBR::NORMAL] = Resources::Load<Texture>(path);
                            }
                        }
                        ImGui::EndDragDropTarget();
                    }
                }

                {
                    ImGui::SeparatorText("Metalica");
                    if (material.texture[TexturePBR::METALLIC] != nullptr && material.texture[TexturePBR::METALLIC]->isvalid()) {
                        ImGui::Image(reinterpret_cast<ImTextureID>(material.texture[TexturePBR::METALLIC]->getID()), ImVec2(32, 32));
                        if (ImGui::IsItemHovered() && ImGui::IsKeyDown(ImGuiKey_X)) {
                            material.texture[TexturePBR::METALLIC] = nullptr;
                        }
                    } else {
                        ImGui::DragFloat("##Metalica", &material.metallic, 0.005f, 0.0f, 1.0f);
                    }

                    if (ImGui::BeginDragDropTarget()) {
                        if (const ImGuiPayload* data = ImGui::AcceptDragDropPayload(getDragDrop(DragDrop::TEXTURE), ImGuiDragDropFlags_None)) {
                            std::string path((char*)data->Data, data->DataSize);
                            if (!path.empty()) {
                                material.texture[TexturePBR::METALLIC] = Resources::Load<Texture>(path);
                            }
                        }
                        ImGui::EndDragDropTarget();
                    }
                }

                {
                    ImGui::SeparatorText("Rugosidade");
                    if (material.texture[TexturePBR::ROUGHNESS] != nullptr && material.texture[TexturePBR::ROUGHNESS]->isvalid()) {
                        ImGui::Image(reinterpret_cast<ImTextureID>(material.texture[TexturePBR::ROUGHNESS]->getID()), ImVec2(32, 32));
                        if (ImGui::IsItemHovered() && ImGui::IsKeyDown(ImGuiKey_X)) {
                            material.texture[TexturePBR::ROUGHNESS] = nullptr;
                        }
                    } else {
                        ImGui::DragFloat("##Rugosidade", &material.roughness, 0.005f, 0.0f, 1.0);
                    }

                    if (ImGui::BeginDragDropTarget()) {
                        if (const ImGuiPayload* data = ImGui::AcceptDragDropPayload(getDragDrop(DragDrop::TEXTURE), ImGuiDragDropFlags_None)) {
                            std::string path((char*)data->Data, data->DataSize);
                            if (!path.empty()) {
                                material.texture[TexturePBR::ROUGHNESS] = Resources::Load<Texture>(path);
                            }
                        }
                        ImGui::EndDragDropTarget();
                    }
                }

                {
                    ImGui::SeparatorText("AO");
                    if (material.texture[TexturePBR::AO] != nullptr && material.texture[TexturePBR::AO]->isvalid()) {
                        ImGui::Image(reinterpret_cast<ImTextureID>(material.texture[TexturePBR::AO]->getID()), ImVec2(32, 32));
                        if (ImGui::IsItemHovered() && ImGui::IsKeyDown(ImGuiKey_X)) {
                            material.texture[TexturePBR::AO] = nullptr;
                        }
                    } else {
                        ImGui::DragFloat("##AO", &material.ao, 0.005f, 0.0f, 1.0);
                    }

                    if (ImGui::BeginDragDropTarget()) {
                        if (const ImGuiPayload* data = ImGui::AcceptDragDropPayload(getDragDrop(DragDrop::TEXTURE), ImGuiDragDropFlags_None)) {
                            std::string path((char*)data->Data, data->DataSize);
                            if (!path.empty()) {
                                material.texture[TexturePBR::AO] = Resources::Load<Texture>(path);
                            }
                        }
                        ImGui::EndDragDropTarget();
                    }
                }
                */
                
                Color clr = {albedo.x, albedo.y, albedo.z, albedo.w};
                if (View::PopupChangeColor("Albedo", clr)) {
                    material.setAlbedo({clr.r, clr.g, clr.b, clr.a});
                }

                ImGui::TreePop();
            }
        }
        
        ImGui::TreePop();
    }
}

/*
void Game::OnPropertyCameraComponent(CameraComponent *cameraComponent) {
    static int itemID = 0;

    if (ImGui::TreeNode("CameraComponent")) {
        ImGui::Text("Fov:");
        ImGui::DragFloat("##CameraComponentFov", &cameraComponent->m_fov, 1.0f, 1.0f, 179.0f);

        ImGui::Text("FarPlane:");
        ImGui::DragFloat("##CameraComponentFarPlane", &cameraComponent->m_farPlane);

        ImGui::Text("NearPlane:");
        ImGui::DragFloat("##CameraComponentNearPlane", &cameraComponent->m_nearPlane);

        ImGui::Text("Projeção:");
        ImGui::Combo("##projection", &itemID, "Perspectiva\0Ortografia\0");
        if (itemID == 0) {
            cameraComponent->m_projection = CameraComponent::ePerspective;
        } else if (itemID == 1) {
            cameraComponent->m_projection = CameraComponent::erthographic;
        }
        
        ImGui::TreePop();
    }
}
*/

void Game::OnPropertyLightComponent(Light *light) {
    if (ImGui::TreeNode("Light")) {
        ImGui::Text("Constante:");
        ImGui::DragFloat("##constant", &light->m_attenuationConstant, 0.005f, 0.01f, 1.0f);

        ImGui::Text("Linear:");
        ImGui::DragFloat("##linear", &light->m_attenuationLinear, 0.005f, 0.01f, 1.0f);

        ImGui::Text("Quadract:");
        ImGui::DragFloat("##quadract", &light->m_attenuationQuadratic, 0.005f, 0.01f, 1.0f);

        ImGui::Text("InConner:");
        ImGui::DragFloat("##inconer", &light->m_angleInnerCone, 0.005f, 0.01f, 1.0f);

        ImGui::Text("IuterConner:");
        ImGui::DragFloat("##outerconner", &light->m_angleOuterCone, 0.005f, 0.01f, 1.0f);

        ImGui::Text("Cor:");
        ImVec4 color = {light->m_color.r, light->m_color.g, light->m_color.b, 1.0f};
        if (ImGui::ColorButton("##LightColor", color)) {
            ImGui::OpenPopup("##LightColorPopup");
        }
        if (ImGui::BeginPopup("##LightColorPopup")) {
            ImGui::ColorPicker4("#LightColorPick", light->m_color.data);
            ImGui::EndPopup();
        }

        ImGui::TreePop();
    }
}

/*
void Game::OnPropertyChunkController(ChunkController *chunkController) {
    if (ImGui::TreeNode("ChunkController")) {
        if (ImGui::Button("Generate")) {
            chunkController->BuildMesh();
        }
        
        ImGui::TreePop();
    }
}
*/

void Game::TextureInfo(Texture *&texture) {
    bool IsOpen = true;

    const ImVec2 textureSize(100, 100);
    const ImVec2 window(300, 400);

    ImGui::SetNextWindowSize(window, ImGuiCond_Appearing);
    ImGui::SetNextWindowPos(ImVec2((Screen::width / 2) - (window.x / 2), (Screen::height / 2) - (window.y / 2)), ImGuiCond_Appearing);
    ImGui::Begin("Texture Info", &IsOpen);

    const float width = ImGui::GetWindowWidth();

    ImGui::SetCursorPosX((width / 2) - (textureSize.x / 2));
    ImGui::Image(reinterpret_cast<ImTextureID>(texture->getID()), textureSize);

    ImGui::Spacing();

    ImGui::SeparatorText("Path");
    ImGui::Text("%s", texture->getpath().c_str());

    ImGui::SeparatorText("Dimensões");
    ImGui::Text("Width: %d", texture->getwidth());
    ImGui::Text("Height: %d", texture->getheight());

    ImGui::SeparatorText("Configurações");
    ImGui::Text("Espaço de cor: %s", Resources::gettextureformatname(texture->getinternalformat()));

    if (!ImGui::IsWindowFocused()) {
        texture = nullptr;
    }
    ImGui::End();

    if (!IsOpen) {
        texture = nullptr;
    }
}

void Game::MenuAdd() {
    ImGui::SetNextWindowPos(ImGui::GetMousePos(), ImGuiCond_Appearing);
    ImGui::SetNextWindowSize(ImVec2(200, 300), ImGuiCond_Appearing);
    ImGui::Begin("##Game::MenuAdd", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
    if (ImGui::TreeNode("Luzes")) {
        if (ImGui::Button("Direct")) {
            // objectselected = SceneManager::add("(Light) Direction");
            //objectselected->addComponent<LightComponent>(LightComponent::eDirection);

            bMenuAdd = false;
        }
        if (ImGui::Button("Point")) {
            // objectselected = SceneManager::add("(Light) Point");
            //objectselected->addComponent<LightComponent>(LightComponent::ePoint);

            bMenuAdd = false;
        }
        if (ImGui::Button("Spot")) {
            // objectselected = SceneManager::add("(Light) Spot");
            //objectselected->addComponent<LightComponent>(LightComponent::eSpot);

            bMenuAdd = false;
        }
        ImGui::TreePop();
    }
    if (!ImGui::IsWindowFocused()) {
        bMenuAdd = false;
    }
    ImGui::End();
}

void Game::PlayGame() {
    ScriptManager::Initialzie();

    RenderManager::Initialize();

    ScreenRender::Initialize();
}

void Game::StopGame() {
    ScriptManager::Shutdown();

    RenderManager::Shutdown();

    ScreenRender::Shutdown();
}

void Game::loadReources() {
    iconsUI.emplace(IconType::ICON_FILE,        Resources::LoadTextureL(Filesystem::GetFile("/icon") + "/icons8-file-30.png"));
    iconsUI.emplace(IconType::ICON_FOLDER,      Resources::LoadTextureL(Filesystem::GetFile("/icon") + "/icons8-folder-50.png"));
    iconsUI.emplace(IconType::ICON_BACK_FILE,   Resources::LoadTextureL(Filesystem::GetFile("/icon") + "/icons8-go-back-50.png"));
    iconsUI.emplace(IconType::ICON_GAMEOBJECT,  Resources::LoadTextureL(Filesystem::GetFile("/icon") + "/icons8-3d-object-50.png"));
    iconsUI.emplace(IconType::ICON_PENCIL,      Resources::LoadTextureL(Filesystem::GetFile("/icon") + "/icons8-lápis-30.png"));
    iconsUI.emplace(IconType::ICON_LIGHT,       Resources::LoadTextureL(Filesystem::GetFile("/icon") + "/icons8-lâmpada-globular-64.png"));
    iconsUI.emplace(IconType::ICON_TEXTURE,     Resources::LoadTextureL(Filesystem::GetFile("/icon") + "/icons8-textura-de-renderização-32.png"));
    iconsUI.emplace(IconType::ICON_PNG,         Resources::LoadTextureL(Filesystem::GetFile("/icon") + "/icons8-png-24.png"));
    iconsUI.emplace(IconType::ICON_PLAY_GAME,   Resources::LoadTextureL(Filesystem::GetFile("/icon") + "/icons8-reproduzir-30.png"));
    iconsUI.emplace(IconType::ICON_STOP_GAME,   Resources::LoadTextureL(Filesystem::GetFile("/icon") + "/icons8-parar-50.png"));
    iconsUI.emplace(IconType::ICON_BIN,         Resources::LoadTextureL(Filesystem::GetFile("/icon") + "/icons8-resíduos-50.png"));
    iconsUI.emplace(IconType::ICON_IMPORT,      Resources::LoadTextureL(Filesystem::GetFile("/icon") + "/icons8-importar-50.png"));
    iconsUI.emplace(IconType::ICON_ASCENDENTE,  Resources::LoadTextureL(Filesystem::GetFile("/icon") + "/icons8-ordenar-para-a-direita-50.png"));
    iconsUI.emplace(IconType::ICON_DESCENDENTE, Resources::LoadTextureL(Filesystem::GetFile("/icon") + "/icons8-ordem-descendente-50.png"));
    iconsUI.emplace(IconType::ICON_EDIT,        Resources::LoadTextureL(Filesystem::GetFile("/icon") + "/icons8-editar-24.png"));

    s_texturePlayGame = iconsUI[IconType::ICON_PLAY_GAME];
}

void Game::ProgressLoad() {
    const ImVec2 windowSize = {300, 200};

    ImVec2 displaySize = s_io->DisplaySize;
    displaySize.x = (displaySize.x / 2.0f) - windowSize.x / 2.0f;
    displaySize.y = (displaySize.y / 2.0f) - windowSize.y / 2.0f;

    ImGui::SetNextWindowPos(displaySize);
    ImGui::SetNextWindowSize(windowSize);
    ImGui::Begin("GameRender...", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove| ImGuiWindowFlags_NoScrollWithMouse);
    ImGui::Text("Carregando UI");

    if (!s_maxProgress) {
        s_maxProgress = Resources::m_processCount;
    }

    u32 current = Resources::m_processCount;
    if (current > s_maxProgress) {
        s_maxProgress = current;
    }

    ImGui::ProgressBar(1.0f - (float)current / s_maxProgress, ImVec2(windowSize.x, 16));

    ImGui::Text("%d/%d", current, s_maxProgress);

    if (current <= 1) {
        s_maxProgress = 0;
    }
    ImGui::End();
}
