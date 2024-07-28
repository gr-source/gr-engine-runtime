#include "Game.h"

#include "timer.h"
#include "screen.h"
#include "memory.h"
#include "skybox.h"
#include "settings.h"
#include "renderer.h"
#include "console.hpp"
#include "gameObject.h"
#include "eventManager.h"
#include "renderManager.h"
#include "editorController.h"

#include "framebuffer/shadowbuffer.h"
#include "framebuffer/screenbuffer.h"
#include "framebuffer/imguibuffer.h"
#include "framebuffer/thumbnails.h"

#include "resources/resources.h"

#include "component/meshFilter.hpp"
#include "component/meshRender.hpp"
#include "component/transform.hpp"
#include "component/camera.hpp"
#include "component/light.hpp"

#include "guizmo/ImGuizmo.h"

#include "ui/ImGuiRender.h"

#include "filesystem/filesystem.h"

#include "animation/skeleton.h"

#include "ui/View.h"

#include "simplexNoise/SimplexNoise.h"

#include "scriptManager/scriptManager.hpp"

#include "eventManager.h"

#include <filesystem>
#include <sstream>
#include <iomanip>

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

#if !IS_GAME
enum class IconType : int {
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

std::unordered_map<IconType, Texture*> iconsUI;
#endif

Game::Game() : bScriptReloaded(false), bMenuAdd(false), objectselected(nullptr), FPSColor(255, 255, 255, 255) {
    #if !IS_GAME
    m_io = nullptr;
    m_guizmoOperation = ImGuizmo::TRANSLATE;
    m_guizmoManipulate = ImGuizmo::WORLD;

    m_io = &ImGui::GetIO();

    loadReources();

    Settings::IsGameRunning = false;
    #endif
}

#if !IS_GAME
void Game::Gui() {
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
    ImGuiIO& io = ImGui::GetIO();

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("GR - GameEngine 3D", nullptr, ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoTitleBar);
    if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
        ImGui::DockSpace(ImGui::GetID("##RenderDocking"), ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);
    }
    ImGui::End();
    ImGui::PopStyleVar();
}

void Game::RenderScenesUI() {
    ImGui::Begin("Cenario", nullptr, ImGuiWindowFlags_NoResize);
    ImGui::SetNextItemOpen(true, ImGuiCond_Appearing);
    if (ImGui::TreeNodeEx("default", ImGuiTreeNodeFlags_SpanFullWidth)) {
        if (ScriptManager::GetAllGameObject) {
            for (auto& gameObject : ScriptManager::GetAllGameObject()) {
                auto& transform = gameObject->getTransform();

                ReadHierarchy(&transform);
            }
        }
        ImGui::TreePop();
    }

    if (ImGui::IsKeyReleased(ImGuiKey_A) && m_io->KeyCtrl) {
        bMenuAdd = !bMenuAdd;
    }
    
    ImGui::End();

    if (bMenuAdd) {
        MenuAdd();
    }
}

void Game::ReadHierarchy(Transform *transform) {
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanFullWidth;

    auto gameObject = transform->entity->getGameObject();
    Transform* root = transform->getRoot();

    if (objectselected == gameObject) {
        flags |= ImGuiTreeNodeFlags_Selected;
    } else {
        flags &= ~ImGuiTreeNodeFlags_Selected;
    }

    if (!root) {
        flags |= ImGuiTreeNodeFlags_Leaf;
    }

    const bool isOpen = ImGui::TreeNodeEx(gameObject->getName().c_str(), flags);
    if (ImGui::IsItemClicked(ImGuiMouseButton_Middle)) {
        objectselected = gameObject;
    }

    if (isOpen) {
        if (root) {
            ReadHierarchy(root);
        }

        ImGui::TreePop();
    }
}

void Game::RenderViewportUI() {
    static Imguibuffer& m_imguibuffer = Imguibuffer::getInstance();
    static ScreenBuffer& m_screenBuffer = ScreenBuffer::getInstance();

    ImGui::Begin("Viewport", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

    ImGuizmo::SetDrawlist();

    ImVec2 size = ImGui::GetWindowSize();

    ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, size.x, size.y);

    static ImVec2 lastSize = ImGui::GetContentRegionAvail();
    ImVec2 curSize = ImGui::GetContentRegionAvail();

    if (lastSize.x != curSize.x || lastSize.y != curSize.y && curSize.x > 5.0f && curSize.y > 5.0f || bScriptReloaded) {
        Settings::ViewportSize = {static_cast<int>(curSize.x), static_cast<int>(curSize.y)};

        auto camera = Camera::Main();
        if (camera)
            camera->setAspectRatio(curSize.x / curSize.y);

        m_screenBuffer.resize({curSize.x, curSize.y});
        
        lastSize = curSize;
        bScriptReloaded = false;
    }

    ImGui::Image(reinterpret_cast<ImTextureID>(m_imguibuffer.gettexture()), ImGui::GetContentRegionAvail(), ImVec2(0, 1), ImVec2(1, 0));

    //
    if (objectselected != nullptr) {
        if (ImGui::IsKeyReleased(ImGuiKey_T)) {
            m_guizmoOperation = ImGuizmo::TRANSLATE;
        }
        if (ImGui::IsKeyReleased(ImGuiKey_R)) {
            m_guizmoOperation = ImGuizmo::ROTATE;
        }
        if (ImGui::IsKeyReleased(ImGuiKey_S)) {
            m_guizmoOperation = ImGuizmo::SCALE;
        }
        if (ImGui::IsKeyReleased(ImGuiKey_Space)) {
            if (m_guizmoManipulate == ImGuizmo::WORLD) {
                m_guizmoManipulate = ImGuizmo::LOCAL;
            } else if (m_guizmoManipulate == ImGuizmo::LOCAL) {
                m_guizmoManipulate = ImGuizmo::WORLD;
            }
        }
        
        Transform& transform = objectselected->getTransform();

        Matrix4x4 model = transform.getglobalmatrix();

        auto camera = Camera::Main();

        if (ImGuizmo::Manipulate(camera->getViewmatrix().data, camera->getPerspesctive().data, m_guizmoOperation, m_guizmoManipulate, model.data)) {
            model.decompose(transform.scale, transform.rotation, transform.position);
        }
    }

    const ImVec2 wSize =  ImGui::GetCursorPos();

    if (Time::FPS > 50) {
        FPSColor = {0, 255, 0, 255};
    } else if (Time::FPS > 25) {
        FPSColor = {255, 255, 0, 255};
    } else if (Time::FPS > 0) {
        FPSColor = {255, 0, 0, 255};
    }

    std::string fps = "FPS: " + std::to_string(Time::FPS);

    ImGui::SetCursorPos(ImVec2(curSize.x - ImGui::CalcTextSize(fps.c_str()).x, ImGui::CalcTextSize(fps.c_str()).y * 2.5f));

    ImGui::TextColored(FPSColor, "%s", fps.c_str());

    ImGui::SetCursorPos(ImVec2(ImGui::GetContentRegionAvail().x / 2 - wSize.x, 32));

    if (ImGui::ImageButton(reinterpret_cast<ImTextureID>(texturePlayGame->getID()), ImVec2(16, 16))) {
        Settings::IsGameRunning =! Settings::IsGameRunning;

        if (Settings::IsGameRunning) {
            texturePlayGame = iconsUI[IconType::ICON_STOP_GAME];

            ScriptManager::Shutdown();

            RenderManager::Shutdown();

            objectselected = nullptr;
        } else {
            texturePlayGame = iconsUI[IconType::ICON_PLAY_GAME];

            ScriptManager::Initialzie();

            RenderManager::Initialize();

            bScriptReloaded = true;
        }
    }

    ImGui::End();
}

void Game::RenderFilesUI() {
    static std::filesystem::path root = Filesystem::getpublic(Filesystem::eResource).string();
    ImGui::Columns(5, "#Collums1", false);

    if (ImGui::Button("Back") && root.has_parent_path()) {
        root = root.parent_path();
    }
    for (auto file : std::filesystem::directory_iterator(root)) {
        ImGui::NextColumn();

        if (file.is_directory()) {
            if (ImGui::Button(file.path().filename().c_str())) {
                root = file.path();
            }
            continue;
        }
        ImGui::Text("%s", file.path().filename().c_str());
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
        auto& logs = Console::GetListLogs();

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
    static ImGuiIO& io = ImGui::GetIO();

    ImGui::Begin("Propriedades", nullptr, ImGuiWindowFlags_NoResize);

    if (ImGui::BeginTabBar("##propertimenubar")) {
        if (ImGui::BeginTabItem("GameObject", nullptr)) {

            if (objectselected != nullptr) {
                ImGui::Image(reinterpret_cast<ImTextureID>(iconsUI[IconType::ICON_GAMEOBJECT]->getID()), ImVec2(16, 16));

                ImGui::SameLine();

                ImGui::Text("[ %s - %s ]", objectselected->getName().c_str(), objectselected->getTag().c_str());

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

                objectselected->RenderProperty<Transform>(&Game::OnPropertyTransform, this);
                objectselected->RenderProperty<MeshRender>(&Game::OnPropertyMeshRender, this);
                /*
                objectselected->RenderProperty<CameraComponent>(&Game::OnPropertyCameraComponent, this);
                */
                // objectselected->RenderProperty<Light>(&Game::OnPropertyLightComponent, this);
                /*
                objectselected->RenderProperty<Animator>(&Game::OnPropertyAnimator, this);
                objectselected->RenderProperty<ChunkController>(&Game::OnPropertyChunkController, this);
                */
            }

            ImGui::EndTabItem();
        }
        
        ImGui::SetNextItemOpen(true, ImGuiCond_FirstUseEver);
        if (ImGui::BeginTabItem("Estatística", nullptr)) {
            ImGui::SeparatorText("Scripts");
            ImGui::Text("Entidades: %ld", Entity::GetEntityCount());
            ImGui::Text("Memoria: %s", getfilesize(Memory::totalMemoryUsage).c_str());

            ImGui::SeparatorText("Cena");
            ImGui::Text("Vertices: %d | %s", gameStatics.vertices, getfilesize(gameStatics.vertices * sizeof(Vertex3D)).c_str());
            ImGui::Text("Indice: %d | %s", gameStatics.indices, getfilesize(gameStatics.indices * sizeof(u32)).c_str());
            ImGui::Text("Eventos: %d", EventManager::getcount());
            ImGui::Text("Triangulos: %.2f", (gameStatics.indices / 3.0f));

            ImGui::SeparatorText("Resources");
            ImGui::Text("Textures: %d", Resources::gettexturecount());
            ImGui::Text("Shaders: %d", Resources::getshadercount());

            ImGui::SeparatorText("Threads");
            ImGui::Text("Update:  %fms  | FPS: %.2f",  gameStatics.updateMs,  (1.0f / gameStatics.updateMs));
            ImGui::Text("Render:  %fms  | FPS: %.2f",  gameStatics.renderMs,  (1.0f / (Time::deltaTime - gameStatics.renderMs)));
            ImGui::Text("Physics: %fms  | FPS: %.2f",  gameStatics.physicsMs, (1.0f / gameStatics.physicsMs));
            ImGui::Text("Script:  %fms  | FPS: %.2f",  gameStatics.scriptMS,  (1.0f / gameStatics.scriptMS));

            ImGui::SeparatorText("Renderização");
            // ImGui::Text("Cameras: %ld",   Camera::Get.size());
            ImGui::Text("Luzes: %ld",     Light::GetListLight().size());
            ImGui::Text("Draw Render: %d/%d", RenderManager::iDrawCount, RenderManager::iDrawRender);
            
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Configuraçoes")) {
            ImGui::Checkbox("Wireframe", &RenderManager::bWireFrame);
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

        ImGui::TreePop();
    }
}

void ReadSkeleton(const Bone* bone) {
    if (!bone) return;

    ImGui::PushID(bone->getname().c_str());

    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanFullWidth;
    if (!bone->haschildren()) {
        flags |= ImGuiTreeNodeFlags_Leaf;
    }

    const bool isOpen = ImGui::TreeNodeEx(bone->getname().c_str(), flags);
    
    if (isOpen) {
        for (const auto& child : bone->getchildren()) {
            ReadSkeleton(&child);
        }
        ImGui::TreePop();
    }
    ImGui::PopID();
}

void Game::OnPropertyAnimator(Animator *animator) {
    if (ImGui::TreeNode("Animator")) {
        /*
        ImGui::Text("Esqueleto:");

        ReadSkeleton(animator->m_skeleton.getroot());

        ImGui::Text("Animations:");

        for (auto& anim : animator->m_animations) {
            ImGui::Text("%s", anim.getname().c_str());
        }
        ImGui::TreePop();
        */
    }
}

void Game::OnPropertyMeshRender(MeshRender *meshRender) {
    if (ImGui::TreeNode("MeshRender")) {
        for (auto&& [idx, material] : meshRender->getMaterials()) {
            if (ImGui::TreeNode(std::string(material.getName() + "(" + std::to_string(idx) + ")").c_str())) {
                
                {
                    ImGui::SeparatorText("Albedo");
                    auto texture = material.getTexture(0);

                    if (texture != nullptr && texture->isValid()) {
                        ImGui::Image(reinterpret_cast<ImTextureID>(texture->getID()), ImVec2(32, 32));
                        if (ImGui::IsItemHovered() && ImGui::IsKeyDown(ImGuiKey_X)) {
                            texture = nullptr;
                        }
                    } else {
                        /*
                        if (View::TextureInputUI(ImColor(material.albedo.r, material.albedo.g, material.albedo.b, material.albedo.a))) {
                            ImGui::OpenPopup("#PopupChangeColorAlbedo");
                        }
                        */
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

                View::PopupChangeColor("Albedo", material.albedo);
                */

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
        /*
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
            ImGui::ColorPicker4("#LightColorPick", lightComponent->m_color.data);
            ImGui::EndPopup();
        }
        */

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

void Game::loadReources() {
    iconsUI.emplace(IconType::ICON_FILE,        Resources::Load<Texture>(Filesystem::getpublic(Filesystem::eIcon).string() + "/icons8-file-30.png"));
    iconsUI.emplace(IconType::ICON_FOLDER,      Resources::Load<Texture>(Filesystem::getpublic(Filesystem::eIcon).string() + "/icons8-folder-50.png"));
    iconsUI.emplace(IconType::ICON_BACK_FILE,   Resources::Load<Texture>(Filesystem::getpublic(Filesystem::eIcon).string() + "/icons8-go-back-50.png"));
    iconsUI.emplace(IconType::ICON_GAMEOBJECT,  Resources::Load<Texture>(Filesystem::getpublic(Filesystem::eIcon).string() + "/icons8-3d-object-50.png"));
    iconsUI.emplace(IconType::ICON_PENCIL,      Resources::Load<Texture>(Filesystem::getpublic(Filesystem::eIcon).string() + "/icons8-lápis-30.png"));
    iconsUI.emplace(IconType::ICON_LIGHT,       Resources::Load<Texture>(Filesystem::getpublic(Filesystem::eIcon).string() + "/icons8-lâmpada-globular-64.png"));
    iconsUI.emplace(IconType::ICON_TEXTURE,     Resources::Load<Texture>(Filesystem::getpublic(Filesystem::eIcon).string() + "/icons8-textura-de-renderização-32.png"));
    iconsUI.emplace(IconType::ICON_PNG,         Resources::Load<Texture>(Filesystem::getpublic(Filesystem::eIcon).string() + "/icons8-png-24.png"));
    iconsUI.emplace(IconType::ICON_PLAY_GAME,   Resources::Load<Texture>(Filesystem::getpublic(Filesystem::eIcon).string() + "/icons8-reproduzir-30.png"));
    iconsUI.emplace(IconType::ICON_STOP_GAME,   Resources::Load<Texture>(Filesystem::getpublic(Filesystem::eIcon).string() + "/icons8-parar-50.png"));
    iconsUI.emplace(IconType::ICON_BIN,         Resources::Load<Texture>(Filesystem::getpublic(Filesystem::eIcon).string() + "/icons8-resíduos-50.png"));
    iconsUI.emplace(IconType::ICON_IMPORT,      Resources::Load<Texture>(Filesystem::getpublic(Filesystem::eIcon).string() + "/icons8-importar-50.png"));
    iconsUI.emplace(IconType::ICON_ASCENDENTE,  Resources::Load<Texture>(Filesystem::getpublic(Filesystem::eIcon).string() + "/icons8-ordenar-para-a-direita-50.png"));
    iconsUI.emplace(IconType::ICON_DESCENDENTE, Resources::Load<Texture>(Filesystem::getpublic(Filesystem::eIcon).string() + "/icons8-ordem-descendente-50.png"));
    iconsUI.emplace(IconType::ICON_EDIT,        Resources::Load<Texture>(Filesystem::getpublic(Filesystem::eIcon).string() + "/icons8-editar-24.png"));

    texturePlayGame = iconsUI[IconType::ICON_PLAY_GAME];
}

void Game::ProgressLoad() {
    ImGuiIO& io = ImGui::GetIO();

    const ImVec2 windowSize = {300, 200};

    ImVec2 displaySize = io.DisplaySize;
    displaySize.x = (displaySize.x / 2.0f) - windowSize.x / 2.0f;
    displaySize.y = (displaySize.y / 2.0f) - windowSize.y / 2.0f;

    ImGui::SetNextWindowPos(displaySize);
    ImGui::SetNextWindowSize(windowSize);
    ImGui::Begin("GameRender...", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove| ImGuiWindowFlags_NoScrollWithMouse);
    ImGui::Text("Carregando UI");

    static u32 maxProgress = Resources::m_processCount;
    u32 current = Resources::m_processCount;
    if (current > maxProgress) {
        maxProgress = current;
    }

    ImGui::ProgressBar(1.0f - (float)current / maxProgress, ImVec2(windowSize.x, 16));

    ImGui::Text("%d/%d", current, maxProgress);

    if (current <= 1) {
        maxProgress = 0;
    }
    ImGui::End();
}

#endif
