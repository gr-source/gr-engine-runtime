#include "editor.hpp"

#include "postProcessing.hpp"
#include "editorController.h"
#include "renderManager.h"
#include "screenRender.hpp"
#include "renderer.h"
#include "window.hpp"
#include "settings.h"
#include "engine.h"
#include "memory.h"
#include "skybox.h"

#include "core/filmbox/filmbox.hpp"

#include "core/binary/model3DHandler.hpp"

#include "core/cmd/cmd.hpp"

#include "core/file/initialization.hpp"
#include "core/file/config.hpp"
#include "core/file/file.hpp"

#include "core/assetsManager.hpp"
#include "core/threadpool.hpp"
#include "core/statistics.hpp"
#include "core/console.hpp"
#include "core/screen.hpp"
#include "core/scene.hpp"
#include "core/event.hpp"
#include "core/timer.hpp"

#include "script/boxCollider.hpp"
#include "script/meshFilter.hpp"
#include "script/meshRender.hpp"
#include "script/transform.hpp"
#include "script/rigidbody.hpp"
#include "script/animator.hpp"
#include "script/camera.hpp"
#include "script/light.hpp"

#include "guizmo/ImGuizmo.h"

#include "ui/ImGuiRender.h"
#include "ui/project.hpp"
#include "ui/view.hpp"

#include "scriptManager/scriptManager.hpp"

#include "geometry/batching.hpp"

#include "ecs/gameObject.hpp"
#include "ecs/manager.hpp"

#include <linux/input-event-codes.h>
#include <filesystem>
#include <sstream>
#include <iomanip>

#include <ecs/componentRegistry.hpp>

#define IMGUI_PAYLOAD_GAMEOBJECT "##payload_gameobject"
#define IMGUI_PAYLOAD_ANIMATION "##payload_animation"
#define IMGUI_PAYLOAD_FILE "##payload_file"
#define IMGUI_PAYLOAD_IMAGE "##playload_image"

#define ITEMS_PER_PAGE 20

std::string getfilesize(std::size_t sz) {
    float kb = sz / 1024.0f;
    float mb = kb / 1024.0f;
    float gb = mb / 1024.0f;

    if (gb >= 1.0f) {
        return std::to_string(gb) + "GB";
    } else if (mb >= 1.0f) {
        return std::to_string(mb) + "MB";
    } else if (kb >= 1.0f) {
        return std::to_string(kb) + "KB";
    }
    return std::to_string(sz) + "B";
}

Editor::Editor() : m_viewport(ImGui::GetMainViewport()), mImGuiIO(&ImGui::GetIO()), m_currentPath(File::GetAsset("")), m_cameraObject(nullptr), m_mainCamera(nullptr), s_objectSelected(nullptr), m_currentPage(0), m_materialToRename(nullptr), tempCam(nullptr), uIPopup(0) {
    s_texturePlayGame = nullptr;

    s_objectSelected = nullptr;

    m_currentPage = 0;

    bFileDoubleClick = false;

    bCutAllFile = false;

    bCopyAllFile = false;

    bViewPortFocused = false;

    bOpenPopupInfoDir = false;

    bScriptReloaded = false;

    bOpenPopupOptionGameObject = false;

    pPopupFileManager = false;

    FPSColor = {1.0f, 1.0f, 1.0f, 1.0f};

    m_lastViewportSize = {0.0f, 0.0f};

    m_guizmoOperation = ImGuizmo::TRANSLATE;

    m_guizmoManipulate = ImGuizmo::WORLD;

    s_maxProgress = 0;

    m_SelectedPath = "";

    mLastTransform = {Vector3::zero, Quaternion::identity, Vector3::one};


    m_config = std::make_shared<Config>(File::GetFile("/config.ini"));
    try {
        m_config->load();
    } catch (const std::exception &e) {
        Console::LogWarning(e.what());
    }

    {
        auto lasCaterogy = m_config->getCategory();
        m_config->setCategory("viewport");
        
        Settings::ViewportSize.x = m_config->hasKey("x") ? m_config->getValue<int>("x") : 0;
        Settings::ViewportSize.y = m_config->hasKey("y") ? m_config->getValue<int>("y") : 0;

        m_config->setCategory(lasCaterogy);
    }

    bBuildDock = File::Exists("imgui.ini");

    LoadReources();

    try {
        ScriptManager::Initialize(File::GetFile("/.build/component/lib/libgr-component.so"));

        if (ScriptManager::SetScreenSize) {
            ScriptManager::SetScreenSize(Settings::ViewportSize);
        }
    } catch (const std::exception &e) {
        Console::LogError("Fatal error in script: " + std::string(e.what()));
    }

    auto lastCategory = m_config->getCategory();

    m_config->setCategory("scene");
    if (m_config->hasKey("main") && m_config->getValue<std::string>("main") != "default" && !Scene::HasScene()) {
        Scene::LoadScene(File::GetAsset("/scene/" + m_config->getValue<std::string>("main") + ".scn"));
    }

    m_config->setCategory(lastCategory);

    if (!File::Exists(File::GetFile("/MainEngineCamera.gmdx", true))) {
        m_cameraObject = CreateEditorCamera();
        if (m_cameraObject) {
            m_mainCamera = m_cameraObject->getComponent<Camera>();
        }
    } else {
        AssetsManager::LoadGameObject(File::GetFile("/MainEngineCamera.gmdx", true), &Editor::loadCamera, this);
    }

    Scene::Initialize();

    RenderManager::Initialize();

    ScreenRender::Initialize();

    Skybox::Initialize();

    Window::SetName("GameEngine: " + Project::GetCurrent()->getName());
}

Editor::~Editor() {
    m_config->setCategory("scene");
    m_config->setValue<std::string>("main", Scene::HasScene() ?  Scene::GetCurrentScene()->getName() : "default");

    m_config->setCategory("viewport");
    m_config->setValue("x", Settings::ViewportSize.x);
    m_config->setValue("y", Settings::ViewportSize.y);

    if (m_config) {
        m_config->save();
    }

    if (m_cameraObject) {
        GameObject::Destroy(m_cameraObject);
    }

    Scene::Shutdown();

    RenderManager::Shutdown();

    Skybox::Shutdown();

    ScreenRender::Shutdown();

    GameObject::Clear();

    ComponentRegistry::Clean();

    ScriptManager::Shutdown();

    Console::Shutdown();

    File::Delete(File::GetTempFile("gr-engine"));
}

void Editor::DropFile(const std::filesystem::path &path) {
    if (path.extension() == ".fbx") {
        return Import3DModel(path.string());
    } else if (path.extension() == ".png" || path.extension() == ".jpg") {
        return ImportImage(path.string());
    }
}
 
void Editor::Import3DModel(const std::string &path) {
    // Threadpool::AddTask(&Filmbox::Load, path, .., false);
    Model3DHandler::ImportFBX(path);
}

void Editor::ImportImage(const std::string &path) {
    auto assets_texture = File::GetAsset("/texture");
    if (!File::Exists(assets_texture)) {
        return Console::LogError("[ Import ] Assets data project do not loaded.");
    }
    Console::LogInfo("[ Import ] Importing texture...");

    if (!File::Copy(path, assets_texture)) {
        return Console::LogError("[ Import ] " + File::GetError());
    }
    Console::LogInfo("[ Import ] Texture imported.");
}

void Editor::KeyBoardUp(grm::u32 key) {
    if (!(bViewPortFocused && key == KEY_ESC)) {
        return;
    }

    if (mImGuiIO)
        mImGuiIO->MouseDrawCursor = !mImGuiIO->MouseDrawCursor;
}

void Editor::LoadScript() {
    if (m_cameraObject) {
        Editor::setActiveCamera(false);
    }
    
    if (ScriptManager::IsValid()) {
        UnloadScript();
    }

    if (ScriptManager::CheckScriptLoaded("libgr-component.so")) {
        return Console::LogError("[ Script ] Fatal error data not unloaded.");
    }

    try {
        ScriptManager::Initialize(File::GetFile("/.build/component/lib/libgr-component.so"));
    } catch (const std::runtime_error &e) {
        Console::LogError(e.what());
    }

    Scene::RequestScript();

    if (ScriptManager::SetScreenSize) {
        ScriptManager::SetScreenSize(Settings::ViewportSize);
    }

    RenderManager::Initialize();

    ScreenRender::Initialize();

    // È necessário ?
    // Scene::UpdateScript();

    Scene::InitializeObject();

    Settings::bGameRunning = true;

    Console::log(Log::DEFAULT, "Game playing.");
}

void Editor::UnloadScript() {
    RenderManager::Shutdown();

    ScreenRender::Shutdown();

    ComponentRegistry::Clean();

    ScriptManager::Shutdown();
}

// ===================================== END =========================================================
void Editor::RenderUI() {
    RenderDocking();

    RenderViewportUI();

    RenderScenesUI();

    RenderOtherUI();

    RenderConsoleWindow();

    RenderPropertiUI();

    if (m_mainCamera && mImGuiIO->KeyCtrl && ImGui::IsKeyReleased(ImGuiKey_S)) {
        for (auto &&[name, scene] : Scene::GetScenes()) {
            Event::Add(&Editor::SaveScene, this, name, &(*scene));
        }
    }

    if (uIPopup & UIPopup_PopupAddComponent) {
        ImGui::OpenPopup("##popup_add_component");
        uIPopup &= ~UIPopup_PopupAddComponent;
    }

    PopupComponentMenu();

    PopupMenuSettings();

    showShaderListPopup();

    showMeshListPopup();

    PopupAddComponent();
}

void Editor::BuildDock(ImGuiID dockspace_id, ImGuiDockNodeFlags dockspace_flags) {
    ImGui::DockBuilderRemoveNode(dockspace_id); // clear any previous layout
	ImGui::DockBuilderAddNode(dockspace_id, dockspace_flags | ImGuiDockNodeFlags_DockSpace);
	ImGui::DockBuilderSetNodeSize(dockspace_id, m_viewport->Size);

	// auto dock_id_center = ImGui::DockBuilderGetCentralNode(dockspace_id);
	auto dock_id_down = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Down, 0.25f, nullptr, &dockspace_id);
	auto dock_id_left = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Left, 0.2f, nullptr, &dockspace_id);
	auto dock_id_right = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Right, 0.15f, nullptr, &dockspace_id);
	auto dock_id_left_down = ImGui::DockBuilderSplitNode(dock_id_down, ImGuiDir_Down, 0.2f, nullptr, &dock_id_down);
	auto dock_id_left_down_right = ImGui::DockBuilderSplitNode(dock_id_left_down, ImGuiDir_Right, 0.2f, nullptr, &dock_id_left_down);

    // Right
	ImGui::DockBuilderDockWindow("Propriedades", dock_id_right);

    // Left
	ImGui::DockBuilderDockWindow("Cenario", dock_id_left);
	ImGui::DockBuilderDockWindow("Navegador", dock_id_left_down);
	ImGui::DockBuilderDockWindow("Console", dock_id_left_down_right);

	ImGui::DockBuilderDockWindow("Viewport", dockspace_id);

	ImGui::DockBuilderFinish(dockspace_id);
}

void Editor::PopupComponentMenu() {
    auto isOpen = ImGui::BeginPopup("##game_object_component_menu");
    if (!isOpen) {
        return;
    }
    ImGui::EndPopup();
}

#include "projectMenu.hpp"

void Editor::RenderDocking() {
    if (m_viewport == nullptr)
        m_viewport = ImGui::GetMainViewport();

    ImGui::SetNextWindowPos(m_viewport->WorkPos);
    ImGui::SetNextWindowSize(m_viewport->WorkSize);
    ImGui::SetNextWindowViewport(m_viewport->ID);

    ImGuiWindowFlags host_window_flags = 0;
    host_window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDocking;
    host_window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    host_window_flags |= ImGuiWindowFlags_MenuBar;

    ImGuiDockNodeFlags dockspace_flags = 0;
    dockspace_flags |= ImGuiDockNodeFlags_PassthruCentralNode;
    if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
        host_window_flags |= ImGuiWindowFlags_NoBackground;

    char label[32];
    ImFormatString(label, IM_ARRAYSIZE(label), "DockSpaceViewport_%08X", m_viewport->ID);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin(label, NULL, host_window_flags);
    ImGui::PopStyleVar(3);

    bool bPopupMenuSettings = false, bPopupMenuShader = false, bMeshList = false;

    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("Project")) {
            if (ImGui::MenuItem("Reconfigure Script")) {
                ReconfigScript();
            }
            
            if (ImGui::MenuItem("Reconfigure Component")) {
                // ProjectMenu::CloseProject();
            }
            
            if (ImGui::MenuItem("Close")) {
                ProjectMenu::CloseProject();
            }
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("View")) {
            if (ImGui::MenuItem("Propriedades")) {
            }
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("Build")) {
            if (ImGui::MenuItem("compile")) {
            }
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("Preferences")) {
            if (ImGui::MenuItem("Settings")) {
                bPopupMenuSettings = true;
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Tools")) {
            if (ImGui::MenuItem("Shader List")) {
                bPopupMenuShader = true;
            }

            if (ImGui::MenuItem("Mesh List")) {
                bMeshList = true;
            }
            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }

    ImGuiID dockspace_id = ImGui::GetID("DockSpace");
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
    ImGui::End();

    if (bPopupMenuSettings) {
        ImGui::OpenPopup("##popup_menu_settings");
    }

    if (bPopupMenuShader) {
        ImGui::OpenPopup("##popup_menu_shader_list");
    }

    if (bMeshList) {
        ImGui::OpenPopup("##popup_menu_mesh_list");
    }

    if (bBuildDock) {
        return;
    }

    BuildDock(dockspace_id, dockspace_flags);

    bBuildDock = true;
}

void Editor::RenderScenesUI() {
    ImGui::Begin("Cenario", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

    auto name = Scene::HasScene() ? Scene::GetCurrentScene()->getName() : "Default";

    ImGui::SetNextItemOpen(true, ImGuiCond_Appearing);
    if (ImGui::TreeNodeEx(name.empty() ? "Scene name is empty!" : name.c_str(), ImGuiTreeNodeFlags_SpanFullWidth)) {
        if (ImGui::BeginDragDropTarget()) {
            ImGuiDragDropFlags target_flags = 0;
            // target_flags |= ImGuiDragDropFlags_AcceptBeforeDelivery;
            // target_flags |= ImGuiDragDropFlags_SourceAllowNullID;

            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(IMGUI_PAYLOAD_GAMEOBJECT, target_flags)) {
                std::string name((char*)payload->Data, payload->DataSize);
                if (!name.empty()) {
                    auto target = GameObject::Find(name);

                    target->getTransform()->setParent(nullptr);
                    
                    Scene::GetCurrentScene()->insert(target);
                }
            }
            ImGui::EndDragDropTarget();
        }
        
        RenderHierarchi();

        ImGui::TreePop();
    }

    if (ImGui::IsKeyReleased(ImGuiKey_Delete) && s_objectSelected) {
        Event::Add(&Scene::Delete, s_objectSelected);
        s_objectSelected = nullptr;
    }
    
    if (ImGui::IsKeyReleased(ImGuiKey_F2) && ImGui::IsWindowHovered() && s_objectSelected) {
        ImGui::OpenPopup("##popup_rename_gameobject");
    }

    if (ImGui::IsKeyReleased(ImGuiKey_Escape) && ImGui::IsWindowHovered()) {
        s_objectSelected = nullptr;
    }

    if (!bOpenPopupOptionGameObject && ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
        ImGui::OpenPopup("##popup_menu_add");
    }

    if (bOpenPopupOptionGameObject) {
        ImGui::OpenPopup("##popup_options_gameobject");
        bOpenPopupOptionGameObject = false;
    }

    PopupOptionsGameObject();

    PopupRenameGameObject();

    PopupMenuAdd();

    ImGui::End();
}

void Editor::RenderHierarchi() {
    if (!Scene::HasScene()) {
        return;
    }

    auto &list = Scene::GetCurrentScene()->getObjectList();

    if (m_currentPage > 0 && ImGui::Button("Before")) {
        --m_currentPage;
    }
    
    RenderPage(list, m_currentPage);
    
    if (list.size() >= m_currentPage && ImGui::Button("Next")) {
        ++m_currentPage;
    }
}

void Editor::RenderPage(std::vector<GameObject *> &list, int page) {
    int start = page * ITEMS_PER_PAGE;
    int end = std::min(start + ITEMS_PER_PAGE, static_cast<int>(list.size()));

    for (int i = start; i < end; ++i) {
        auto transform = list[i]->getTransform();

        ReadHierarchy(transform);
    }
}

void Editor::ReadHierarchy(Transform *transform) {
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Selected | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanFullWidth;

    auto gameObject = transform->getGameObject();
    if (!gameObject) {
        return;
    }
    auto name = gameObject->getName();

    if (s_objectSelected != gameObject) {
        flags &= ~ImGuiTreeNodeFlags_Selected;
    }

    if (transform->hasChildren()) {
        flags &= ~ImGuiTreeNodeFlags_Leaf;
    }

    auto isOpen = ImGui::TreeNodeEx(name.c_str(), flags);

    if (ImGui::IsItemClicked(ImGuiMouseButton_Left) && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
        s_objectSelected = gameObject;
    }

    if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
        s_objectSelected = gameObject;
        bOpenPopupOptionGameObject = true;
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
                if (!target) {
                    Console::LogError("GameObject do not found: " + name);
                } else {
                    auto ts = target->getTransform();
                    if (ts != transform->getParent()) {
                        ts->setParent(transform);
                    
                        Event::Add(&Scene::Remove, target);
                    }
                }
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

void Editor::PopupMenuAdd() {
    auto isOpen = ImGui::BeginPopup("##popup_menu_add");
    if (!isOpen) {
        return;
    }
    const char* item_name[2] {"Cube", "Empty"};
    auto id = 0;

    ImGui::Text("Add Object");
    if (ImGui::ListBox("##popup_menu_menu_add_list_box", &id, item_name, (sizeof(item_name) / sizeof(item_name[0])))) {
        switch (id) {
        case 0:
            s_objectSelected = CreatePrimitive(0);
            break;
        case 1:
            s_objectSelected = Scene::GetCurrentScene()->CreateGameObject("Empty");
            break;
        default:
            break;
        }
        ImGui::CloseCurrentPopup();
    }

    ImGui::EndPopup();
}

void Editor::PopupOptionsGameObject() {
    if (!s_objectSelected || !m_mainCamera) {
        return;
    }

    bool isOpenRename = false;
    if (ImGui::BeginPopup("##popup_options_gameobject")) {
        std::vector<const char *> item_name {"Delete", "Rename", "Look at me"};

        auto id = 0;
        if (ImGui::ListBox("##ListBoxOptionsGame", &id, item_name.data(), item_name.size())) {
            switch (id) {
            case 0:
                Event::Add(&Scene::Delete, s_objectSelected);
                s_objectSelected = nullptr;
                break;
            case 1:
                isOpenRename = true;
                break;
            case 2:
                EditorController::LookAt(s_objectSelected->getTransform());
            default:
                break;
            }
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    if (isOpenRename && s_objectSelected) {
        ImGui::OpenPopup("##popup_rename_gameobject");
    }
}

void Editor::PopupRenameGameObject() {
    auto isOpen = ImGui::BeginPopup("##popup_rename_gameobject");
    if (!isOpen) {
        return;
    }
    ImGui::Text("Input new name");

    if (!ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked(0))
        ImGui::SetKeyboardFocusHere(0);

    auto name = s_objectSelected->getName();
    if (name.size() < 256) {
        name.resize(256, '\0');
    }
    
    if (ImGui::InputText("##input_new_name_gameobject", name.data(), name.size(), ImGuiInputTextFlags_EnterReturnsTrue)) {
        name.erase(std::remove(name.begin(), name.end(), '\0'), name.end());

        GameObject::Rename(s_objectSelected, name);

        ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
}

void Editor::RenderViewportUI() {
    ImGui::Begin("Viewport", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    
    auto isOpen = false;

    bViewPortFocused = ImGui::IsWindowFocused();

    if (ImGui::BeginTabBar("##SceneListTabBar")) {
        auto &scenes = Scene::GetScenes();

        for (auto it = scenes.begin(); it != scenes.end();) {
            auto& [name, scene] = *it;

            ImGuiTabItemFlags flags = ImGuiTabItemFlags_None;

            auto cache_scene = File::GetTempFile("gr-engine/" + name + ".scn.tmp");
            if (File::Exists(cache_scene)) {
                flags |= ImGuiTabItemFlags_UnsavedDocument;
            }
            
            bool isQuit = true;
            if (ImGui::BeginTabItem(name.c_str(), &isQuit, flags)) {
                if (Scene::GetCurrentScene() != &(*scene)) {
                    Scene::Use(&(*scene));

                    s_objectSelected = nullptr;
                }
                    
                RenderScene();
                ImGui::EndTabItem();
            }

            if (!isQuit) {
                s_objectSelected = nullptr;
                Scene::Use(nullptr);
                
                it = scenes.erase(it);
                continue;
            }
            ++it;
        }

        if (ImGui::TabItemButton("+")) {
            isOpen = true;
        }
        ImGui::EndTabBar();
    }

    if (isOpen) {
        ImGui::OpenPopup("##popup_menu_create_scene");
    }

    DropSceneInViewport();

    PopupMenuCreateScene();

    ImGui::End();
}

void Editor::DropSceneInViewport() {
    auto isOpen = ImGui::BeginDragDropTarget();
    if (!isOpen) {
        return;
    }

    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(IMGUI_PAYLOAD_FILE)) {
        std::string path((char*)payload->Data, payload->DataSize);
        if (!path.empty() && std::filesystem::path(path).filename().extension() == ".scn") {
            Scene::LoadScene(path);
        }
    }
    ImGui::EndDragDropTarget();
}

bool gluInvertMatrix(const float m[16], float invOut[16]) {
    float inv[16], det;
    int i;

    inv[0] = m[5]  * m[10] * m[15] - 
             m[5]  * m[11] * m[14] - 
             m[9]  * m[6]  * m[15] + 
             m[9]  * m[7]  * m[14] +
             m[13] * m[6]  * m[11] - 
             m[13] * m[7]  * m[10];

    inv[4] = -m[4]  * m[10] * m[15] + 
              m[4]  * m[11] * m[14] + 
              m[8]  * m[6]  * m[15] - 
              m[8]  * m[7]  * m[14] - 
              m[12] * m[6]  * m[11] + 
              m[12] * m[7]  * m[10];

    inv[8] = m[4]  * m[9] * m[15] - 
             m[4]  * m[11] * m[13] - 
             m[8]  * m[5] * m[15] + 
             m[8]  * m[7] * m[13] + 
             m[12] * m[5] * m[11] - 
             m[12] * m[7] * m[9];

    inv[12] = -m[4]  * m[9] * m[14] + 
               m[4]  * m[10] * m[13] +
               m[8]  * m[5] * m[14] - 
               m[8]  * m[6] * m[13] - 
               m[12] * m[5] * m[10] + 
               m[12] * m[6] * m[9];

    inv[1] = -m[1]  * m[10] * m[15] + 
              m[1]  * m[11] * m[14] + 
              m[9]  * m[2] * m[15] - 
              m[9]  * m[3] * m[14] - 
              m[13] * m[2] * m[11] + 
              m[13] * m[3] * m[10];

    inv[5] = m[0]  * m[10] * m[15] - 
             m[0]  * m[11] * m[14] - 
             m[8]  * m[2] * m[15] + 
             m[8]  * m[3] * m[14] + 
             m[12] * m[2] * m[11] - 
             m[12] * m[3] * m[10];

    inv[9] = -m[0]  * m[9] * m[15] + 
              m[0]  * m[11] * m[13] + 
              m[8]  * m[1] * m[15] - 
              m[8]  * m[3] * m[13] - 
              m[12] * m[1] * m[11] + 
              m[12] * m[3] * m[9];

    inv[13] = m[0]  * m[9] * m[14] - 
              m[0]  * m[10] * m[13] - 
              m[8]  * m[1] * m[14] + 
              m[8]  * m[2] * m[13] + 
              m[12] * m[1] * m[10] - 
              m[12] * m[2] * m[9];

    inv[2] = m[1]  * m[6] * m[15] - 
             m[1]  * m[7] * m[14] - 
             m[5]  * m[2] * m[15] + 
             m[5]  * m[3] * m[14] + 
             m[13] * m[2] * m[7] - 
             m[13] * m[3] * m[6];

    inv[6] = -m[0]  * m[6] * m[15] + 
              m[0]  * m[7] * m[14] + 
              m[4]  * m[2] * m[15] - 
              m[4]  * m[3] * m[14] - 
              m[12] * m[2] * m[7] + 
              m[12] * m[3] * m[6];

    inv[10] = m[0]  * m[5] * m[15] - 
              m[0]  * m[7] * m[13] - 
              m[4]  * m[1] * m[15] + 
              m[4]  * m[3] * m[13] + 
              m[12] * m[1] * m[7] - 
              m[12] * m[3] * m[5];

    inv[14] = -m[0]  * m[5] * m[14] + 
               m[0]  * m[6] * m[13] + 
               m[4]  * m[1] * m[14] - 
               m[4]  * m[2] * m[13] - 
               m[12] * m[1] * m[6] + 
               m[12] * m[2] * m[5];

    inv[3] = -m[1] * m[6] * m[11] + 
              m[1] * m[7] * m[10] + 
              m[5] * m[2] * m[11] - 
              m[5] * m[3] * m[10] - 
              m[9] * m[2] * m[7] + 
              m[9] * m[3] * m[6];

    inv[7] = m[0] * m[6] * m[11] - 
             m[0] * m[7] * m[10] - 
             m[4] * m[2] * m[11] + 
             m[4] * m[3] * m[10] + 
             m[8] * m[2] * m[7] - 
             m[8] * m[3] * m[6];

    inv[11] = -m[0] * m[5] * m[11] + 
               m[0] * m[7] * m[9] + 
               m[4] * m[1] * m[11] - 
               m[4] * m[3] * m[9] - 
               m[8] * m[1] * m[7] + 
               m[8] * m[3] * m[5];

    inv[15] = m[0] * m[5] * m[10] - 
              m[0] * m[6] * m[9] - 
              m[4] * m[1] * m[10] + 
              m[4] * m[2] * m[9] + 
              m[8] * m[1] * m[6] - 
              m[8] * m[2] * m[5];

    det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];

    if (det == 0)
        return false;

    det = 1.0 / det;

    for (i = 0; i < 16; i++)
        invOut[i] = inv[i] * det;

    return true;
}

/*
bool gluInvertMatrix(const Matrix4x4 &m, Matrix4x4 &invOut) {
    Matrix4x4 inv = Matrix4x4::identityMatrix;

    inv[0][1] = -m[0][1] * m[2][2] * m[3][3] +
                m[0][1] * m[2][3] * m[3][2] +
                m[0][2] * m[2][1] * m[3][3] -
                m[0][2] * m[2][3] * m[3][1] -
                m[0][3] * m[2][1] * m[3][2] +
                m[0][3] * m[2][2] * m[3][1];

    inv[0][2] = m[0][1] * m[1][2] * m[3][3] -
                m[0][1] * m[1][3] * m[3][2] -
                m[0][2] * m[1][1] * m[3][3] +
                m[0][2] * m[1][3] * m[3][1] +
                m[0][3] * m[1][1] * m[3][2] -
                m[0][3] * m[1][2] * m[3][1];

    inv[0][3] = -m[0][1] * m[1][2] * m[2][3] +
                 m[0][1] * m[1][3] * m[2][2] +
                 m[0][2] * m[1][1] * m[2][3] -
                 m[0][2] * m[1][3] * m[2][1] -
                 m[0][3] * m[1][1] * m[2][2] +
                 m[0][3] * m[1][2] * m[2][1];

    inv[1][0] = -m[1][0] * m[2][2] * m[3][3] +
                 m[1][0] * m[2][3] * m[3][2] +
                 m[1][2] * m[2][0] * m[3][3] -
                 m[1][2] * m[2][3] * m[3][0] -
                 m[1][3] * m[2][0] * m[3][2] +
                 m[1][3] * m[2][2] * m[3][0];

    inv[1][1] = m[0][0] * m[2][2] * m[3][3] -
                m[0][0] * m[2][3] * m[3][2] -
                m[0][2] * m[2][0] * m[3][3] +
                m[0][2] * m[2][3] * m[3][0] +
                m[0][3] * m[2][0] * m[3][2] -
                m[0][3] * m[2][2] * m[3][0];

    inv[1][2] = -m[0][0] * m[1][2] * m[3][3] +
                 m[0][0] * m[1][3] * m[3][2] +
                 m[0][2] * m[1][0] * m[3][3] -
                 m[0][2] * m[1][3] * m[3][0] -
                 m[0][3] * m[1][0] * m[3][2] +
                 m[0][3] * m[1][2] * m[3][0];

    inv[1][3] = m[0][0] * m[1][2] * m[2][3] -
                m[0][0] * m[1][3] * m[2][2] -
                m[0][2] * m[1][0] * m[2][3] +
                m[0][2] * m[1][3] * m[2][0] +
                m[0][3] * m[1][0] * m[2][2] -
                m[0][3] * m[1][2] * m[2][0];

    inv[2][0] = m[1][0] * m[2][1] * m[3][3] -
                m[1][0] * m[2][3] * m[3][1] -
                m[1][1] * m[2][0] * m[3][3] +
                m[1][1] * m[2][3] * m[3][0] +
                m[1][3] * m[2][0] * m[3][1] -
                m[1][3] * m[2][1] * m[3][0];

    inv[2][1] = -m[0][0] * m[2][1] * m[3][3] +
                 m[0][0] * m[2][3] * m[3][1] +
                 m[0][1] * m[2][0] * m[3][3] -
                 m[0][1] * m[2][3] * m[3][0] -
                 m[0][3] * m[2][0] * m[3][1] +
                 m[0][3] * m[2][1] * m[3][0];

    inv[2][2] = m[0][0] * m[1][1] * m[3][3] -
                m[0][0] * m[1][3] * m[3][1] -
                m[0][1] * m[1][0] * m[3][3] +
                m[0][1] * m[1][3] * m[3][0] +
                m[0][3] * m[1][0] * m[3][1] -
                m[0][3] * m[1][1] * m[3][0];

    inv[2][3] = -m[0][0] * m[1][1] * m[2][3] +
                 m[0][0] * m[1][3] * m[2][1] +
                 m[0][1] * m[1][0] * m[2][3] -
                 m[0][1] * m[1][3] * m[2][0] -
                 m[0][3] * m[1][0] * m[2][1] +
                 m[0][3] * m[1][1] * m[2][0];

    inv[3][0] = -m[1][0] * m[2][1] * m[3][2] +
                 m[1][0] * m[2][2] * m[3][1] +
                 m[1][1] * m[2][0] * m[3][2] -
                 m[1][1] * m[2][2] * m[3][0] -
                 m[1][2] * m[2][0] * m[3][1] +
                 m[1][2] * m[2][1] * m[3][0];

    inv[3][1] = m[0][0] * m[2][1] * m[3][2] -
                m[0][0] * m[2][2] * m[3][1] -
                m[0][1] * m[2][0] * m[3][2] +
                m[0][1] * m[2][2] * m[3][0] +
                m[0][2] * m[2][0] * m[3][1] -
                m[0][2] * m[2][1] * m[3][0];

    inv[3][2] = -m[0][0] * m[1][1] * m[3][2] +
                 m[0][0] * m[1][2] * m[3][1] +
                 m[0][1] * m[1][0] * m[3][2] -
                 m[0][1] * m[1][2] * m[3][0] -
                 m[0][2] * m[1][0] * m[3][1] +
                 m[0][2] * m[1][1] * m[3][0];

    inv[3][3] = m[0][0] * m[1][1] * m[2][2] -
                m[0][0] * m[1][2] * m[2][1] -
                m[0][1] * m[1][2] * m[2][0] -
                m[0][2] * m[1][0] * m[2][1] +
                m[0][2] * m[1][1] * m[2][0];


    auto det = m[0][0] * inv[0][0] + m[0][1] * inv[1][0] + m[0][2] * inv[2][0] + m[0][3] * inv[3][0];

    if (det == 0.0f) {
        return false;
    }
    
    det = 1.0 / det;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            invOut[i][j] = inv[i][j] * det;
        }
    }

    return true;
}
*/

#include "core/input.hpp"

void Editor::RenderScene() {
    auto cursorY = ImGui::GetCursorPosY();
    cursorY += 5.0f;

    ImVec2 regionAvail = ImGui::GetContentRegionAvail();
    if (m_mainCamera && (m_lastViewportSize.x != regionAvail.x || m_lastViewportSize.y != regionAvail.y && regionAvail.x > 5.0f && regionAvail.y > 5.0f || bScriptReloaded)) {
        auto &viewport = Settings::ViewportSize = {static_cast<int>(regionAvail.x), static_cast<int>(regionAvail.y)};

        if (ScriptManager::SetScreenSize) {
            ScriptManager::SetScreenSize(Settings::ViewportSize);
        }

        m_mainCamera->setAspectRatio(static_cast<float>(viewport.x) / static_cast<float>(viewport.y));

        Engine::GetMainPostprocess()->resize(viewport);
        
        m_lastViewportSize = regionAvail;
        bScriptReloaded = false;
    }

    auto texture = Engine::GetImGuiPostprocess()->getTextureColor();
    if (texture) {
        ImGui::Image(reinterpret_cast<ImTextureID>(texture->getID()), regionAvail, {0.0f, 1.0f}, {1.0f, 0.0f});

        if (ImGui::IsWindowHovered()) {
            ImVec2 mousePos = ImGui::GetMousePos();
            ImVec2 windowPos = ImGui::GetWindowPos();
            ImVec2 localMousePos = ImVec2(mousePos.x - windowPos.x, mousePos.y - windowPos.y);

            if (localMousePos.x >= 0 && localMousePos.x <= regionAvail.x &&
                localMousePos.y >= 0 && localMousePos.y <= regionAvail.y) {
                    /*
                    if (localMousePos.x < 10.f) {
                        Input::SetCursorPos({mousePos.x + regionAvail.x, mousePos.y});
                    } else if (localMousePos.x > regionAvail.x - 10.0f) {
                        Input::SetCursorPos({mousePos.x - (regionAvail.x - 10.0f), mousePos.y});
                    }
                    */
            }
        }

        DropGameObjectInScene();
    }

    auto cursorPos =  ImGui::GetCursorPos();
    auto pos = ImGui::GetWindowPos();

    // Play button
    if (s_texturePlayGame) {
        s_texturePlayGame = iconsUI[ICON_PLAY_GAME];
        if (Settings::bGameRunning) {
            if (ImGui::IsKeyReleased(ImGuiKey_Escape) && ImGui::IsWindowHovered()) {
                StopGame();
            } else {
                s_texturePlayGame = iconsUI[ICON_STOP_GAME];
            }
        }
        
        ImGui::SetCursorPos({regionAvail.x / 2 - cursorPos.x, cursorY});

        if (ImGui::ImageButton(reinterpret_cast<ImTextureID>(s_texturePlayGame->getID()), ImVec2(16, 16))) {
            if (!Settings::bGameRunning) {
                PlayGame();
            } else {
                StopGame();
            }
        }
    }

    ImGuizmo::SetDrawlist();
    ImGuizmo::SetRect(pos.x, pos.y + (cursorY - 5.0f), regionAvail.x, regionAvail.y);

    if (m_mainCamera && s_objectSelected && s_objectSelected->hasComponent<Transform>()) {
        RenderGuizmo();
    }

    if (TimeLast::FPS > 50) {
        FPSColor = {0, 255, 0, 255};
    } else if (TimeLast::FPS > 25) {
        FPSColor = {255, 255, 0, 255};
    } else if (TimeLast::FPS > 0) {
        FPSColor = {255, 0, 0, 255};
    }

    // Fps Draw
    {
        std::string fps = "FPS: 000.00f";

        auto textSize = ImGui::CalcTextSize(fps.c_str());

        auto curtorPos = ImVec2(regionAvail.x - textSize.x, cursorY + textSize.y);

        ImGui::SetCursorPos(curtorPos);

        ImGui::TextColored(FPSColor, "FPS: %.2f", Statistics::m_threadFrame.fps);
    }

    ImGui::SetCursorPos({8.0f, cursorY});

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

    if (s_objectSelected && !mImGuiIO->MouseDrawCursor && ImGui::IsWindowFocused() && mImGuiIO->KeyShift && ImGui::IsKeyReleased(ImGuiKey_D)) {
        s_objectSelected = GameObject::Instantiate(s_objectSelected);
    }
}

void Editor::PlayGame() {
    Threadpool::AddTask(&Editor::BuildScript, this);
}

void Editor::StopGame() {
    Settings::bGameRunning = false;

    Scene::DestroyScript();

    Scene::SetupEssential();

    Editor::setActiveCamera(true);

    Console::log(Log::DEFAULT, "Game stoped.");
}

void Editor::loadCamera(GameObject *gameObject) {
    m_cameraObject = gameObject;
    if (!m_cameraObject) {
        m_cameraObject = CreateEditorCamera();
    }

    m_mainCamera = m_cameraObject->getComponent<Camera>();

    m_cameraObject->initialize(false);
}

void Editor::RenderGuizmo() {
    if (!mImGuiIO->MouseDrawCursor && !mImGuiIO->KeyShift && !mImGuiIO->KeyCtrl && ImGui::IsKeyReleased(ImGuiKey_G) && ImGui::IsWindowHovered()) {
        m_guizmoOperation = ImGuizmo::TRANSLATE;
    }
    
    if (!mImGuiIO->MouseDrawCursor && !mImGuiIO->KeyShift && !mImGuiIO->KeyCtrl && ImGui::IsKeyReleased(ImGuiKey_R) && ImGui::IsWindowHovered()) {
        m_guizmoOperation = ImGuizmo::ROTATE;
    }
    
    if (!mImGuiIO->MouseDrawCursor && !mImGuiIO->KeyShift && !mImGuiIO->KeyCtrl && ImGui::IsKeyReleased(ImGuiKey_S) && ImGui::IsWindowHovered()) {
        m_guizmoOperation = ImGuizmo::SCALE;
    }

    if (mImGuiIO->KeyCtrl && ImGui::IsKeyReleased(ImGuiKey_Space) && ImGui::IsWindowHovered()) {
        if (m_guizmoManipulate == ImGuizmo::WORLD) {
            m_guizmoManipulate = ImGuizmo::LOCAL;
        } else if (m_guizmoManipulate == ImGuizmo::LOCAL) {
            m_guizmoManipulate = ImGuizmo::WORLD;
        }
    }
    
    auto transform = s_objectSelected->getTransform();

    auto globalMatrix = transform->getGlobalMatrix();

    if (m_mainCamera && ImGuizmo::Manipulate(m_mainCamera->getViewmatrix().getData(), m_mainCamera->getPerspesctive().getData(), m_guizmoOperation, m_guizmoManipulate, globalMatrix.getData())) {
        auto localMatrix = globalMatrix;
        if (transform->hasParent()) {
            auto parentMat = transform->getParent()->getGlobalMatrix();
            Matrix4x4 m = Matrix4x4::identityMatrix;

            if (gluInvertMatrix(parentMat.getData(), m.getData()))
                localMatrix = m * localMatrix;
        }
        Vector3 s;
        Quaternion r;
        Vector3 t;
        localMatrix.decompose(s, r, t);

        transform->setLocalScale(s);
        transform->setLocalRotation(r);
        transform->setLocalPosition(t);
    }
}

void Editor::DropGameObjectInScene() {
    auto isOpen = ImGui::BeginDragDropTarget();
    if (!isOpen) {
        return;
    }

    auto payload = ImGui::AcceptDragDropPayload(IMGUI_PAYLOAD_FILE);
    if (!payload) {
        return ImGui::EndDragDropTarget();
    }

    std::string path(reinterpret_cast<char *>(payload->Data), payload->DataSize);
    if (path.empty()) {
        return ImGui::EndDragDropTarget();
    }
    
    if (std::filesystem::path(path).filename().extension() == ".gmdx") {
        AssetsManager::LoadGameObject(path, &Editor::PostLoadGameObject, this);
    }
    
    ImGui::EndDragDropTarget();
}

void Editor::PopupMenuCreateScene() {
    auto isOpen = ImGui::BeginPopup("##popup_menu_create_scene");
    if (!isOpen) {
        return;
    }

    ImGui::Text("Input scene name");

    if (!ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked(ImGuiMouseButton_Left))
        ImGui::SetKeyboardFocusHere();

    std::string name(256, '\0');
    if (ImGui::InputText("##popup_create_scene_inputtext", name.data(), name.size(), ImGuiInputTextFlags_EnterReturnsTrue)) {
        name.erase(std::remove(name.begin(), name.end(), '\0'), name.end());

        if (!name.empty()) {
            Scene::Create(name);
        }
        
        ImGui::CloseCurrentPopup();
    }

    ImGui::EndPopup();
}

void Editor::RenderFilesUI() {
    ImGui::SetNextItemOpen(true, ImGuiCond_FirstUseEver);
    RenderTreeNodeFile(m_currentPath);

    if (pPopupFileManager) {
        ImGui::OpenPopup("##popup_dir_manager");
        pPopupFileManager = false;
    }

    PopupDirmanager();

    PopupCreateDirectory();

    PopupCreateScript();

    PopupRenameFile();
}

void Editor::RenderTreeNodeFile(std::filesystem::path path) {
    if (path.filename().string()[0] == '.') {
        return;
    }
    
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf;
    flags |= ImGuiTreeNodeFlags_SpanFullWidth;
    flags |= ImGuiTreeNodeFlags_SpanAvailWidth;
    flags |= ImGuiTreeNodeFlags_Selected;

    auto bDirectory = std::filesystem::is_directory(path);
    if (bDirectory) {
        flags &= ~ImGuiTreeNodeFlags_Leaf;
    }

    if (fLisDirSelected.find(path) == fLisDirSelected.end()) {
        flags &= ~ImGuiTreeNodeFlags_Selected;
    }

    auto name = path.filename();
    
    auto isOpen = ImGui::TreeNodeEx(name.c_str(), flags);

    if (ImGui::IsWindowFocused() && ImGui::IsKeyReleased(ImGuiKey_Escape)) {
        fLisDirSelected.clear();
    }

    if (ImGui::IsItemClicked() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
        OpenFileDoubleClick(path);
    }

    if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
        if (!(flags & ImGuiTreeNodeFlags_Selected)) {
            fLisDirSelected.emplace(path, true);
        }
        pPopupFileManager = true;
    } else if (mImGuiIO->KeyShift && ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
        fLisDirSelected.emplace(path, true);
    } else if (ImGui::IsItemClicked(ImGuiMouseButton_Left) && fLisDirSelected.find(path) == fLisDirSelected.end()) {
        fLisDirSelected.clear();
    } else if (ImGui::IsItemClicked(ImGuiMouseButton_Left) && fLisDirSelected.find(path) != fLisDirSelected.end()) {
        fLisDirSelected.clear();
        fLisDirSelected.emplace(path, true);
    }

    DropTargetGameObjectInFile(path.string());

    if (ImGui::BeginDragDropTarget()) {
        ImGuiDragDropFlags target_flags = 0;
        // target_flags |= ImGuiDragDropFlags_AcceptBeforeDelivery;
        // target_flags |= ImGuiDragDropFlags_SourceAllowNullID;

        if (auto payload = ImGui::AcceptDragDropPayload(IMGUI_PAYLOAD_FILE, target_flags)) {
            std::filesystem::path from = std::string((char*)payload->Data, payload->DataSize);
            if (!from.empty() && (path.string() + "/" + from.filename().string()) != from.string()) {
                File::Move(from, path);
            }
        }
        ImGui::EndDragDropTarget();
    }

    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceNoDisableHover | ImGuiDragDropFlags_SourceNoHoldToOpenOthers)) {
        auto str = path.string();

        auto bAnimation = path.extension().string() == ".anm";

        ImGui::Text("%s", str.c_str());
        ImGui::SetDragDropPayload(bAnimation ? IMGUI_PAYLOAD_ANIMATION : IMGUI_PAYLOAD_FILE, str.data(), str.size());
        ImGui::EndDragDropSource();
    }

    if (!isOpen) {
        return;
    }

    if (!bDirectory) {
        return ImGui::TreePop();
    }

    std::vector<std::filesystem::directory_entry> dirs;
    for (auto &file : std::filesystem::directory_iterator(path)) {
        dirs.push_back(file);
    }

    std::sort(dirs.begin(), dirs.end(), [](const auto& lhs, const auto& rhs) {
        if (lhs.is_directory() != rhs.is_directory()) {
            return lhs.is_directory() > rhs.is_directory();
        }
        return lhs.path().filename() < rhs.path().filename();
    });

    for (auto &file : dirs) {
        RenderTreeNodeFile(file);
    }
    ImGui::TreePop();
}

void Editor::DropTargetGameObjectInFile(std::string path) {
    if (!ImGui::BeginDragDropTarget()) {
        return;
    }
    
    auto payload = ImGui::AcceptDragDropPayload(IMGUI_PAYLOAD_GAMEOBJECT);
    if (!payload) {
        return ImGui::EndDragDropTarget();
    }

    std::string name(static_cast<char *>(payload->Data), payload->DataSize);
    if (name.empty()) {
        return ImGui::EndDragDropTarget();
    }
    
    auto target = GameObject::Find(name);
    if (!target) {
        Console::LogError("GameObject do not found: %s", name.c_str());

        return ImGui::EndDragDropTarget();
    }

    path = path[path.size()] == '/' ? path : path + "/";

    Model3DHandler::ExportGRMX(path + name + ".gmdx", target);
    
    ImGui::EndDragDropTarget();
}

void Editor::OpenFileDoubleClick(std::filesystem::path path) {
    if (path.extension().string() != ".hpp" && path.extension().string() != ".cpp") {
        return;
    }

    Command::exec("code '" + m_currentPath.string() + "'");
    Command::exec("code '" + path.string() + "'");
}

void Editor::PopupDirmanager() {
    auto isOpen = ImGui::BeginPopup("##popup_dir_manager");
    if (!isOpen) {
        return;
    }

    for (auto &&[path, isSeleceted] : fLisDirSelected) {
        ImGui::Text("Dir: %s", path.filename().string().c_str());

        m_SelectedPath = path;
    }

    auto idx = 0;
    bool bCreateDiretory = false;
    bool bCreateScript = false;
    bool bRenameFile = false;

    std::array<const char *, 4> item_list{"Delete", "Rename", "Create directory", "Create script"};

    if (ImGui::ListBox("##DirManager", &idx, item_list.data(), item_list.size())) {
        switch (idx) {
        case 0:
            for (auto &&[path, isSeleceted] : fLisDirSelected) {
                File::Delete(path);
            }
            break;
        case 1:
            bRenameFile = true;
            break;
        case 2:
            bCreateDiretory = true;
            break;
        case 3:
            bCreateScript = true;
            break;
        default:
            break;
        }
        fLisDirSelected.clear();
        ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();

    if (bRenameFile) {
        ImGui::OpenPopup("##popup_rename_file");
    }

    if (bCreateDiretory) {
        ImGui::OpenPopup("##popup_create_new_dir");
    }

    if (bCreateScript) {
        ImGui::OpenPopup("##popup_create_new_script");
    }
}

void Editor::PopupFileManager() {
    bool isOpenPopup = false;
    if (ImGui::BeginPopup("##popup_file_manager")) {
        ImGui::Text("FileManager");

        auto idx = 0;

        const char *item_list[2] {"Create New Directory"};

        if (ImGui::ListBox("##FileManager", &idx, item_list, 1)) {
            switch (idx) {
            case 0:
                isOpenPopup = true;
                break;
            default:
                break;
            }
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    if (isOpenPopup) {
        ImGui::OpenPopup("##popup_create_new_dir");
    }
}

void Editor::PopupCreateScript() {
    auto isOpen = ImGui::BeginPopup("##popup_create_new_script");
    if (!isOpen) {
        return;
    }

    ImGui::Text("Input your script name");

    char name[256] = {0};
    if (ImGui::InputText("##input_text_popup_create_script_name", name, 256, ImGuiInputTextFlags_EnterReturnsTrue)) {
        CreateScriptTemplate(m_SelectedPath.string(), name);

        ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
}

void Editor::PopupCreateDirectory() {
    auto isOpen = ImGui::BeginPopup("##popup_create_new_dir");
    if (!isOpen) {
        return;
    }
    
    ImGui::Text("Input directory name:");
    if (!ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked(0))
        ImGui::SetKeyboardFocusHere(0);

    char name[256] = {0};
    if (ImGui::InputText("##InputDirectoryName", name, 256, ImGuiInputTextFlags_EnterReturnsTrue)) {
        File::CreateDirectory(m_SelectedPath.string() + "/" + std::string(name));
        ImGui::CloseCurrentPopup();
    }

    ImGui::Text("%s/%s", m_SelectedPath.c_str(), name);

    ImGui::EndPopup();
}

void Editor::RenderOtherUI() {
    static NavegationFlags flags = NavegationFlags_files;

    ImGui::Begin("Navegador", nullptr, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_AlwaysHorizontalScrollbar | ImGuiWindowFlags_AlwaysAutoResize);
    
    if (ImGui::BeginMenuBar()) {
        if (ImGui::MenuItem("Files")) {
            flags = NavegationFlags_files;
        }
        if (ImGui::MenuItem("Models")) {
            flags = NavegationFlags_models;
        }
        ImGui::EndMenuBar();
    }

    if (flags & NavegationFlags_files) {
        RenderFilesUI();
    }

    if (flags & NavegationFlags_models) {
    }

    ImGui::End();
}

void Editor::RenderConsoleWindow() {
    ImGui::Begin("Console");
    RenderConsole();
    ImGui::End();
}

void Editor::RenderConsole() {
    auto logs = Console::GetListLogs();

    std::for_each(logs.begin(), logs.end(), std::bind(&Editor::RenderLog, this, std::placeholders::_1));
    
    if (Console::bChanged) {
        ImGui::SetScrollHereY(1.0f);
        Console::bChanged = false;
    }
}

void Editor::RenderLog(Log &info) {
    ImVec4 color = {1.0f, 1.0f, 1.0f, 1.0f};
    switch (info.type) {
    case Log::LogType::INFO:
        color = {0.0f, 1.0f, 0.0f, 1.0f};
        break;
    case Log::LogType::WARNING:
        color = {1.0f, 1.0f, 0.0f, 1.0f};
        break;
    case Log::LogType::ERROR:
        color = {1.0f, 0.0f, 0.0f, 1.0f};
        break;
    default:
        break;
    }

    ImGui::TextColored(color, "%s", info.message.c_str());
}

void Editor::RenderSettings() {
    ImGui::SeparatorText("Render");
    ImGui::Checkbox("Wireframe", &RenderManager::bWireFrame);
    ImGui::Checkbox("Debug Mode", &DebugDraw::bDebugMode);

    m_config->setCategory("config");

    ImGui::SeparatorText("Editor");

    {
        const char *editor_name[] {"None", "Visual Studio Code", "Sublime Text"};
        int len = sizeof(editor_name) / sizeof(editor_name[0]);
    
        int item = m_config->hasKey("use_editor") ? m_config->getValue<int>("use_editor") : m_config->setValue<int>("use_editor", 0);
        if (ImGui::Combo("##render_settings_list_box", &item, editor_name, len)) {
            m_config->setValue<int>("use_editor", item);
    
            switch (item) {
            case 1: // Visual Studio Code
                return ConfigureVSCode();
            }
            ImGui::CloseCurrentPopup();
        }
    
    }
    
    ImGui::SeparatorText("World");
    {
        auto bFpsLimited = m_config->hasKey("fps_limit") ? m_config->getValue<bool>("fps_limit") : m_config->setValue<bool>("fps_limit", true);
        if (ImGui::Checkbox("Fps Limited", &bFpsLimited)) {
            m_config->setValue<bool>("fps_limit", bFpsLimited);
        }
    }

    ImGui::SeparatorText("View");

    auto regionAvail = ImGui::GetContentRegionAvail();
    if (ImGui::Button("Estatisticas", ImVec2{regionAvail.x, 0.0f})) {
    }
}

void Editor::ConfigureVSCode() {
    Console::LogInfo("Initializing Visual Studio Code Setup...");
    
    auto internal_vscode = File::GetFile("/config/vscode/c_cpp_properties.json", true);
    if (!File::Exists(internal_vscode)) {
        return Console::LogError("Falied to load internal data config.");
    }

    auto header = File::GetFile("/.build/component/include");
    if (!File::Exists(header)) {
        return Console::LogError("Project do not configured.");
    }

    auto source = File::GetFileContent(internal_vscode);
    if (source.empty()) {
        return Console::LogError("Falied to read internal data.");
    }

    source = String::replace_all(source, "${header_internal_script}", header);

    auto dest_vscode = File::GetAsset("/.vscode");
    if (File::Exists(dest_vscode)) {
        File::Delete(dest_vscode);
    }

    if (!File::CreateDirectory(dest_vscode)) {
        return Console::LogError("Falied to create config in project.");
    }

    std::ofstream os(File::GetAsset("/.vscode/c_cpp_properties.json"));
    os << source << std::endl;
    os.close();

    Console::LogWarning("Visual Studio Code has been configured.");
}

void Editor::RenderPropertiUI() {
    ImGui::Begin("Propriedades", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

    if (ImGui::BeginTabBar("##render_properti_ui_tabbar")) {
        RenderGameObjectPropertiUI();
        
        /*
        if (ImGui::BeginTabItem("Estatística", nullptr)) {
            RenderStatisticsMenu();
            ImGui::EndTabItem();
        }
        */
        ImGui::EndTabBar();
    }
    ImGui::End();
}

void Editor::RenderStatisticsMenu() {
    ImGui::SeparatorText("Scripts");
    ImGui::Text("Entidades: %ld", Manager::GetEntityCount());
    ImGui::Text("GameObject: %ld", GameObject::GetGameObjectCount());

    ImGui::SeparatorText("AssetsManager");
    ImGui::Text("Textures: %ld", AssetsManager::GetTextureCount());
    ImGui::Text("Shaders: %ld", AssetsManager::GetShaderCount());
    ImGui::Text("Mesh: %ld", AssetsManager::GetMeshCount());
    ImGui::Text("Event: %ld", Event::GetCount());

    ImGui::SeparatorText("Threads");
    ImGui::Text("Threads: %ld/%ld", Threadpool::GetThreadCount(), Threadpool::GetThreadAvailable());

    ImGui::SeparatorText("FrameTime");
    ImGui::Text("MainThread:  %fms  | FPS: %.2f",  Statistics::m_threadFrame.delta,  (1.0f / Statistics::m_threadFrame.delta));
    ImGui::Text("Render:      %fms  | FPS: %.2f",  Statistics::m_sceneFrame.delta,   (1.0f / Statistics::m_sceneFrame.delta));
    ImGui::Text("GUI:         %fms  | FPS: %.2f",  Statistics::m_guiFrame.delta,     (1.0f / Statistics::m_guiFrame.delta));
    ImGui::Text("Script:      %fms  | FPS: %.2f",  Statistics::m_scriptFrame.delta,  (1.0f / Statistics::m_scriptFrame.delta));

    ImGui::SeparatorText("Renderização");
    if (ScriptManager::GetListLight)
        ImGui::Text("Luzes: %ld",     ScriptManager::GetListLight().size());
    ImGui::Text("Draw Call Count: %ld", Statistics::m_drawCallCount);
    ImGui::Text("Draw Render: %ld/%ld", Statistics::m_totalRender, Statistics::m_drawRenderCount);
    ImGui::Text("Batched: %ld", Batching::GetBatchCount());
}

void Editor::RenderGameObjectPropertiUI() {
    if (!s_objectSelected) { return; }

    auto objName = s_objectSelected->getName();
    ImGui::PushID(objName.c_str());

    ImGui::Image(reinterpret_cast<ImTextureID>(iconsUI[IconType::ICON_GAMEOBJECT]->getID()), ImVec2(16, 16));

    ImGui::SameLine();

    ImGui::Text("[ %s - %s ]", objName.c_str(), s_objectSelected->getTag().c_str());

    ImGui::SameLine();

    auto isActive = s_objectSelected->isActive();
    if (ImGui::Checkbox("##check_box_is_active", &isActive)) {
        s_objectSelected->setActive(isActive);
    }

    auto &components = s_objectSelected->getEntity()->getComponents();
    for (auto &&[name, component] : components) {
        ImGui::PushID(name.c_str());
        
        if (name == "Transform") {
            s_objectSelected->RenderProperty<Transform>(&Editor::OnPropertyTransform, this);
            ImGui::PopID();
            continue;
        }
        
        if (name == "Light") {
            if (s_objectSelected->RenderProperty<Light>(&Editor::OnPropertyLight, this)) {
                Event::Add(&Entity::remove, s_objectSelected->getEntity(), name);
            }
            ImGui::PopID();
            continue;
        }

        if (name == "MeshRender") {
            if (s_objectSelected->RenderProperty<MeshRender>(&Editor::OnPropertyMeshRender, this)) {
                Event::Add(&Entity::remove, s_objectSelected->getEntity(), name);
                Event::Add(&Entity::remove, s_objectSelected->getEntity(), "MeshFilter");
            }
            ImGui::PopID();
            continue;
        }

        if (name == "MeshFilter") {
            if (s_objectSelected->RenderProperty<MeshFilter>(&Editor::OnPropertyMeshFilter, this)) {
                Event::Add(&Entity::remove, s_objectSelected->getEntity(), name);
            }
            ImGui::PopID();
            continue;
        }

        if (name == "BoxCollider") {
            if (s_objectSelected->RenderProperty<BoxCollider>(&Editor::OnPropertyBoxCollider, this)) {
                Event::Add(&Entity::remove, s_objectSelected->getEntity(), name);
            }
            ImGui::PopID();
            continue;
        }

        if (name == "Rigidbody") {
            if (s_objectSelected->RenderProperty<Rigidbody>(&Editor::OnPropertyRigidbody, this)) {
                Event::Add(&Entity::remove, s_objectSelected->getEntity(), name);
            }
            ImGui::PopID();
            continue;
        }

        if (name == "Animator") {
            if (s_objectSelected->RenderProperty<Animator>(&Editor::OnPropertyAnimator, this)) {
                Event::Add(&Entity::remove, s_objectSelected->getEntity(), name);
            }
            ImGui::PopID();
            continue;
        }
        
        if (!RenderComponents(name, &(*component))) {
            Event::Add(&Entity::remove, s_objectSelected->getEntity(), name);
            ImGui::PopID();
            continue;
        }
        ImGui::PopID();
    }

    auto avail = ImGui::GetContentRegionAvail();

    if (ImGui::Button("AddComponent", {avail.x, 0.0f})) {
        uIPopup |= UIPopup_PopupAddComponent;
    }

    ImGui::PopID();
}

bool Editor::RenderComponents(const std::string &name, Component *component) {
    auto componentPool = ComponentRegistry::GetPoolList();

    auto isRender = true;

    auto isOpen = ImGui::CollapsingHeader(name.c_str(), &isRender);
    if (!isRender) {
        return false;
    }

    if (!isOpen) {
        return true;
    }

    auto it = componentPool.find(name);
    if (it == componentPool.end()) {
        Console::LogError("Invalid component: " + name);
        return false;
    }
    
    auto fieldList = it->second->getFields();

    for (auto &field : fieldList) {
        RenderField(field, component);
    }

    return true;
}

void Editor::RenderField(Field &field, Component *component) {
    auto name = field.getName();
    auto type = field.getType();

    ImGui::Text("%s:", name.c_str());
    name = "##" + name;
    
    switch (type) {
    case FieldType_float: {
        auto data = Field::GetFieldData<float>(component, field);

        ImGui::DragFloat(name.c_str(), data);
        break;
    }
    case FieldType_int: {
        auto data = Field::GetFieldData<int>(component, field);

        ImGui::DragInt(name.c_str(), data);
        break;
    }
    case FieldType_bool: {
        auto data = Field::GetFieldData<bool>(component, field);

        ImGui::Checkbox(name.c_str(), data);
        break;
    }
    case FieldType_Vector2: {
        auto data = Field::GetFieldData<Vector3>(component, field);

        ImGui::DragFloat2(name.c_str(), reinterpret_cast<float *>(data));
        break;
    }
    case FieldType_Vector3: {
        auto data = Field::GetFieldData<Vector3>(component, field);

        ImGui::DragFloat3(name.c_str(), reinterpret_cast<float *>(data));
        break;
    }
    case FieldType_Vector4: {
        auto data = Field::GetFieldData<Vector3>(component, field);

        ImGui::DragFloat4(name.c_str(), reinterpret_cast<float *>(data));
        break;
    }
    default:
        break;
    }
}

void AddComponentForAll(GameObject *gameObject, std::string name) {
    auto entt = gameObject->getEntity();

    if (!entt->hasComponent(name)) {
        entt->addComponent(name);
    }

    auto transform = gameObject->getTransform();
    for (auto &child : transform->getChildren()) {
        AddComponentForAll(child->getGameObject(), name);
    }
}

void Editor::PopupAddComponent() {
    if (!s_objectSelected) {
        return;
    }
    
    auto isOpen = ImGui::BeginPopup("##popup_add_component");
    if (!isOpen) {
        return;
    }

    std::vector<std::string> m_componentName;
    for (auto [name, pool] : ComponentRegistry::GetPoolList()) {
        m_componentName.push_back(name);
    }

    std::sort(m_componentName.begin(), m_componentName.end(), [](const auto &lhs, const auto &rhs) {
        return lhs < rhs;
    });

    if (!ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked(ImGuiMouseButton_Left))
        ImGui::SetKeyboardFocusHere();

    static char componentName[256] = {'\0'};
    ImGui::InputText("##popup_add_componnet_inputtext_filter", componentName, sizeof(componentName));

    m_componentName.erase(std::remove_if(m_componentName.begin(), m_componentName.end(), [] (auto &name) {
        return std::strstr(name.c_str(), componentName) == nullptr;
    }), m_componentName.end());

    auto regionAvail = ImGui::GetContentRegionAvail();

    for (auto &name : m_componentName) {
        if (ImGui::Button(name.c_str(), ImVec2{regionAvail.x, 0.0f}) && !s_objectSelected->getEntity()->hasComponent(name)) {
            auto component = s_objectSelected->getEntity()->addComponent(name);
            if (component && name == "Light") {
                component->Init();
            }

            std::memset(componentName, '\0', sizeof(componentName));

            ImGui::CloseCurrentPopup();
        }
    }
    
    ImGui::EndPopup();
}

/* ********** Propriedades ********** */
bool Editor::OnPropertyTransform(Transform *transform) {
    auto isOpen = ImGui::CollapsingHeader("Transform");
    if (!isOpen) {
        return false;
    }

    const char *data[3] = {"X:", "Y:", "Z:"};
    
    auto position = transform->getLocalPosition();
    ImGui::Text("Position");
    if (View::DragScalarEx(data, (float *)&position, 3, 1.0f, 0.0f, 0.0f)) {
        transform->setLocalPosition(position);
    }
    
    auto scale = transform->getLocalScale();
    ImGui::Text("Scale");
    if (View::DragScalarEx(data, (float *)&scale, 3, 1.0f, 0.0f, 0.0f)) {
        transform->setLocalScale(scale);
    }

    auto euler = Math::eulerAngles(transform->getLocalRotation());
    ImGui::Text("Rotation");
    if (View::DragScalarEx(data, (float *)&euler, 3, 1.0f, -360.0f, 360.0f)) {
        euler.y = std::clamp(euler.y, -89.0f, 89.0f);

        transform->setLocalRotation(Math::normalize(Math::euler({Math::rad(euler)})));
    }
    
    return false;
}

bool Editor::OnPropertyMeshFilter(MeshFilter *meshFilter) {
    auto bExists = true;

    auto isOpen = ImGui::CollapsingHeader("MeshFilter", &bExists);
    if (!bExists) {
        return true;
    }

    if (!isOpen) {
        return false;
    }

    RenderUVConfig(meshFilter);

    return false;
}

void Editor::RenderUVConfig(MeshFilter *meshFilter) {
    auto mesh = meshFilter->getMesh();

    auto isOpen = ImGui::TreeNode("UV");
    if (!isOpen || !mesh) {
        return;
    }
    const char *name[] {"X", "Y", "Z"};

    ImGui::Text("%s", mesh->getName().c_str());

    ImGui::Text("Position:");
    Vector2 position = Vector2::zero;
    if (View::DragScalarEx(name, reinterpret_cast<float *>(&position), 2, 1.0f, 0.0f, 0.0f)) {
        //
    }

    ImGui::Text("Rotation:");
    Vector3 euler = Vector3::zero;
    if (View::DragScalarEx(name, reinterpret_cast<float *>(&euler), 2, 1.0f, 0.0f, 0.0f)) {
        //
    }

    ImGui::Text("Scale:");
    Vector2 scale = Vector2::zero;
    if (View::DragScalarEx(name, reinterpret_cast<float *>(&scale), 2, 1.0f, 0.0f, 0.0f)) {
        //
    }

    ImGui::TreePop();
}

bool Editor::OnPropertyMeshRender(MeshRender *meshRender) {
    auto bVisible = true;

    auto isOpen = ImGui::CollapsingHeader("MeshRender", &bVisible);
    if (!bVisible) {
        return true;
    }
    
    if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
        ImGui::OpenPopup("##game_object_component_menu");
    }

    if (!isOpen) {
        return false;
    }
   
    auto &materialList = meshRender->getMaterials();
    
    if (ImGui::Button("Create")) {
        materialList.push_back(std::make_unique<Material>());
    }

    ImGui::Indent();
    for (auto it=materialList.begin();it!=materialList.end();) {
        if (!RenderMaterial(it->get())) {
            it++;
        } else {
            it = materialList.erase(it);
        }
    }
    ImGui::Unindent();

    if (m_materialToRename) {
        ImGui::OpenPopup("##popup_rename_material");
    }

    PopupRenameMaterial();
    return false;
}

bool Editor::RenderMaterial(Material *material) {
    auto bVisible = true;

    auto isOpen = ImGui::CollapsingHeader(material->getName().c_str(), &bVisible);
    if (!bVisible) {
        return true;
    }
    
    if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
        m_materialToRename = &(*material);
    }

    if (!isOpen) {
        return false;
    }

    auto albedo = material->getAlbedo();
    InsertTexture(&(*material));

    if (View::PopupChangeColor("Albedo", albedo)) {
        material->setAlbedo(albedo);
    }

    ImGui::Text("Roghness:");
    auto roughness = material->getRoughness();
    if (ImGui::DragFloat("##mesh_render_material_roughness", &roughness, 0.001f)) {
        material->setRoughness(roughness);
    }

    ImGui::Text("Mettallic:");
    auto metallic = material->getMetallic();
    if (ImGui::DragFloat("##mesh_render_material_metallic", &metallic, 0.001f)) {
        material->setMetallic(metallic);
    }

    ImGui::Text("AO:");
    auto ao = material->getAmbientOcclusion();
    if (ImGui::DragFloat("##mesh_render_material_ao", &ao, 0.001f)) {
        material->setAmbientOcclusion(ao);
    }
    return false;
}

void Editor::InsertTexture(Material *material) {
    auto color = material->getAlbedo();
    ImGui::SeparatorText("Albedo");
    
    auto texture = material->getTexture(0);
    if (!texture) {
        if (View::TextureInputUI(ImColor(color.r, color.g, color.b, color.a))) {
            ImGui::OpenPopup("#PopupChangeColorAlbedo");
        }
    } else {
        ImGui::Image(reinterpret_cast<ImTextureID>(texture->getID()), ImVec2(Settings::IconSize.x, Settings::IconSize.y));
    }

    if (!ImGui::BeginDragDropTarget()) {
        return;
    }
    auto data = ImGui::AcceptDragDropPayload(IMGUI_PAYLOAD_FILE, ImGuiDragDropFlags_None);
    if (!data) {
        return ImGui::EndDragDropTarget();
    }

    std::string path((char*)data->Data, data->DataSize);
    if (path.empty()) {
        Console::LogError("[ GUI ] Texture do not loaded, patg is empty.");
        return ImGui::EndDragDropTarget();
    }
    AssetsManager::LoadTexture(path, &Editor::SetupTextureMaterial, this, material, 0);
    ImGui::EndDragDropTarget();
}

bool Editor::OnPropertyLight(Light *light) {
    auto bExists = true;

    auto isOpen = ImGui::CollapsingHeader("Light", &bExists);
    if (!bExists) {
        return true;
    }

    if (!isOpen) {
        return false;
    }

    int elem = light->getType();
    
    const char* elems_names[3] = {"Directional", "Point", "Spot"};
    
    const char* elem_name = (elem >= 0 && elem < 3) ? elems_names[elem] : "Unknown";

    ImGui::Text("Type");
    if (ImGui::SliderInt("##light_type", &elem, 0, 3 - 1, elem_name)) {
        light->setType(elem);
    }
    
    auto value = light->getAttenuationConstant();
    ImGui::Text("Constante");
    if (ImGui::DragFloat("##light_constant", &value, 0.005f, 0.01f, 1.0f)) {
        light->setAttenuationConstant(value);
    }

    value = light->getAttenuationLinear();

    ImGui::Text("Linear:");
    if (ImGui::DragFloat("##light_linear", &value, 0.005f, 0.01f, 1.0f)) {
        light->setAttenuationLinear(value);
    }

    value = light->getAttenuationQuadractc();

    ImGui::Text("Quadract:");
    if (ImGui::DragFloat("##light_quadract", &value, 0.005f, 0.01f, 1.0f)) {
        light->setAttenuationQuadractc(value);
    }

    value = light->getInnecone();

    ImGui::Text("InConner:");
    if (ImGui::DragFloat("##light_inconer", &value, 0.005f, 0.01f, 1.0f)) {
        light->setInnecone(value);
    }

    value = light->getOutercone();

    ImGui::Text("IuterConner:");
    if (ImGui::DragFloat("##light_outerconner", &value, 0.005f, 0.01f, 1.0f)) {
        light->setOutercone(value);
    }

    ImGui::Text("Color");
    
    auto color = light->getColor();
    if (ImGui::ColorButton("##light_color", *reinterpret_cast<ImVec4 *>(&color))) {
        ImGui::OpenPopup("##light_color_popup");
    }
    
    if (ImGui::BeginPopup("##light_color_popup")) {
        if (ImGui::ColorPicker4("#light_color_pick", reinterpret_cast<float *>(&color))) {
            light->setColor(color);
        }
        ImGui::EndPopup();
    }

    return false;
}

bool Editor::OnPropertyBoxCollider(BoxCollider *boxCollider) {
    auto bExists = true;

    auto isOpen = ImGui::CollapsingHeader("BoxCollider", &bExists);
    if (!bExists) {
        return true;
    }

    if (!isOpen) {
        return false;
    }

    const char* text[] {"X", "Y", "Z"};

    auto size = boxCollider->getBoxSize();
    ImGui::Text("Size");
    if (ImGui::DragFloatEx("##boxcollider_size", text, reinterpret_cast<float *>(&size), 1.0f, 0.0f, 0.0f, "%.3f")) {
        boxCollider->setBoxSize(size);
    }


    return false;
}

bool Editor::OnPropertyRigidbody(Rigidbody *rigidbody) {
    auto bExists = true;

    auto isOpen = ImGui::CollapsingHeader("Rigidbody", &bExists);
    if (!bExists) {
        return true;
    }
    
    if (!isOpen) {
        return false;
    }
    auto mass = rigidbody->getMass();

    ImGui::Text("Mass");
    if (ImGui::DragFloat("##rigidbody_mass", &mass)) {
        rigidbody->setMass(mass);
    }

    const char* text[] {"X", "Y", "Z"};

    auto freese = rigidbody->getLookRotation();
    ImGui::Text("Freeze Rotation");
    if (ImGui::DragFloatEx("##rigidbody_freeze", text, reinterpret_cast<float *>(&freese), 1.0f, 0.0f, 0.0f, "%.3f")) {
        rigidbody->setLookRotation(freese);
    }

    ImGui::SeparatorText("Continuous Collision Detection");

    auto fMotionThreshold = rigidbody->getMotionThreshold();
    ImGui::Text("Distance");
    if (ImGui::DragFloat("##rigidbody_ccd_distance", &fMotionThreshold, 0.4f, 0.0f, 5.0f)) {
        rigidbody->setMotionThreshold(fMotionThreshold);
    }

    auto fSweptSphereRadius = rigidbody->getSweptSphereRadius();
    ImGui::Text("Sphere Radius");
    if (ImGui::DragFloat("##rigidbody_ccd_sphereradius", &fSweptSphereRadius, 0.4f, 0.0f, 5.0f)) {
        rigidbody->setSweptSphereRadius(fSweptSphereRadius);
    }

    return false;
}

bool Editor::OnPropertyAnimator(Animator *animator) {
    auto bVisible = true;

    auto bOpen = ImGui::CollapsingHeader("Animator", &bVisible);

    if (!bVisible) {
        return true;
    }

    if (!bOpen) {
        return false;
    }

    auto armature = animator->getArmature();
    auto armature_name = armature != nullptr ? armature->getName() : "No Armature";

    ImGui::Text("Armature: %s", armature_name.c_str());

    if (ImGui::BeginDragDropTarget()) {
        ImGuiDragDropFlags target_flags = 0;
        target_flags |= ImGuiDragDropFlags_AcceptBeforeDelivery;
        target_flags |= ImGuiDragDropFlags_SourceAllowNullID;

        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(IMGUI_PAYLOAD_GAMEOBJECT, target_flags)) {
            std::string name((char*)payload->Data, payload->DataSize);
            if (!name.empty()) {
                animator->setArmature(GameObject::Find(name));
            }
        }
        ImGui::EndDragDropTarget();
    }

    auto animation = animator->getAnimation();
    auto animName = animation != nullptr ? animation->getName() : "No Animation";

    ImGui::Text("Animation: %s", animName.c_str());

    if (ImGui::BeginDragDropTarget()) {
        ImGuiDragDropFlags target_flags = 0;
        target_flags |= ImGuiDragDropFlags_AcceptBeforeDelivery;
        target_flags |= ImGuiDragDropFlags_SourceAllowNullID;

        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(IMGUI_PAYLOAD_ANIMATION, target_flags)) {
            std::string path((char*)payload->Data, payload->DataSize);
            if (!path.empty()) {
                animator->setAnimation(AssetsManager::LoadAnimation(path));
            }
        }
        ImGui::EndDragDropTarget();
    }
    
    return false;
}

void Editor::TextureInfo(Texture *&texture) {
    bool IsOpen = true;

    const ImVec2 textureSize(100, 100);
    const ImVec2 window(300, 400);

    ImGui::SetNextWindowSize(window, ImGuiCond_Appearing);
    ImGui::SetNextWindowPos(ImVec2((Screen::width / 2) - (window.x / 2), (Screen::height / 2) - (window.y / 2)), ImGuiCond_Appearing);
    ImGui::Begin("Texture Info", &IsOpen);

    const float width = ImGui::GetWindowWidth();

    // ImGui::SetCursorPosX((width / 2) - (textureSize.x / 2));
    // ImGui::Image(reinterpret_cast<ImTextureID>(texture->getID()), textureSize);

    // ImGui::Spacing();

    // ImGui::SeparatorText("Path");
    // ImGui::Text("%s", texture->getpath().c_str());

    // ImGui::SeparatorText("Dimensões");
    // ImGui::Text("Width: %d", texture->getwidth());
    // ImGui::Text("Height: %d", texture->getheight());

    ImGui::SeparatorText("Configurações");
    // ImGui::Text("Espaço de cor: %s", Resources::gettextureformatname(texture->getinternalformat()));

    if (!ImGui::IsWindowFocused()) {
        texture = nullptr;
    }
    ImGui::End();

    if (!IsOpen) {
        texture = nullptr;
    }
}

void Editor::LoadReources() {
    AssetsManager::LoadTexture(File::GetAsset("/icon/icons8-file-30.png", true),                    &Editor::SetupTexture, this, IconType::ICON_FILE);
    AssetsManager::LoadTexture(File::GetAsset("/icon/icons8-folder-50.png", true),                  &Editor::SetupTexture, this, IconType::ICON_FOLDER);
    AssetsManager::LoadTexture(File::GetAsset("/icon/icons8-go-back-50.png", true),                 &Editor::SetupTexture, this, IconType::ICON_BACK_FILE);
    AssetsManager::LoadTexture(File::GetAsset("/icon/icons8-3d-object-50.png", true),               &Editor::SetupTexture, this, IconType::ICON_GAMEOBJECT);
    AssetsManager::LoadTexture(File::GetAsset("/icon/icons8-lápis-30.png", true),                   &Editor::SetupTexture, this, IconType::ICON_PENCIL);
    AssetsManager::LoadTexture(File::GetAsset("/icon/icons8-lâmpada-globular-64.png", true),        &Editor::SetupTexture, this, IconType::ICON_LIGHT);
    AssetsManager::LoadTexture(File::GetAsset("/icon/icons8-textura-de-renderização-32.png", true), &Editor::SetupTexture, this, IconType::ICON_TEXTURE);
    AssetsManager::LoadTexture(File::GetAsset("/icon/icons8-png-24.png", true),                     &Editor::SetupTexture, this, IconType::ICON_PNG);
    AssetsManager::LoadTexture(File::GetAsset("/icon/icons8-reproduzir-30.png", true),              &Editor::SetupTexture, this, IconType::ICON_PLAY_GAME);
    AssetsManager::LoadTexture(File::GetAsset("/icon/icons8-parar-50.png", true),                   &Editor::SetupTexture, this, IconType::ICON_STOP_GAME);
    AssetsManager::LoadTexture(File::GetAsset("/icon/icons8-resíduos-50.png", true),                &Editor::SetupTexture, this, IconType::ICON_BIN);
    AssetsManager::LoadTexture(File::GetAsset("/icon/icons8-importar-50.png", true),                &Editor::SetupTexture, this, IconType::ICON_IMPORT);
    AssetsManager::LoadTexture(File::GetAsset("/icon/icons8-ordenar-para-a-direita-50.png", true),  &Editor::SetupTexture, this, IconType::ICON_ASCENDENTE);
    AssetsManager::LoadTexture(File::GetAsset("/icon/icons8-ordem-descendente-50.png", true),       &Editor::SetupTexture, this, IconType::ICON_DESCENDENTE);
    AssetsManager::LoadTexture(File::GetAsset("/icon/icons8-editar-24.png", true),                  &Editor::SetupTexture, this, IconType::ICON_EDIT);
    AssetsManager::LoadTexture(File::GetAsset("/icon/motion-graphics.png", true),                   &Editor::SetupTexture, this, IconType::ICON_ANIMATION);
    AssetsManager::LoadTexture(File::GetAsset("/icon/shade.png", true),                             &Editor::SetupTexture, this, IconType::ICON_SHADER);
    AssetsManager::LoadTexture(File::GetAsset("/icon/studio-set.png", true),                        &Editor::SetupTexture, this, IconType::ICON_SCENE);
}

void Editor::SetupTexture(IconType type, grr::gTexture *texture) {
    iconsUI[type] = texture;

    if (type == IconType::ICON_PLAY_GAME) {
        s_texturePlayGame = iconsUI[IconType::ICON_PLAY_GAME];
    }
}

// TODO: Corrigir
float Editor::GetFps() {
    if (!m_config) {
        return -1.0f;
    }
    m_config->setCategory("config");
    if (!m_config || !m_config->hasKey("fps_limit")) {
        return -1.0f;
    }
    return m_config->getValue<bool>("fps_limit") ? 60.0f : -1.0f;
}

Camera *Editor::getCamera() {
    if (!Camera::Current || !Camera::Current()) {
        return m_mainCamera;
    }
    return Settings::bGameRunning ? Camera::Current() : m_mainCamera;
}

bool Editor::ProgressLoad(int max, int current) {
    const ImVec2 windowSize(300, 200);

    ImVec2 displaySize = mImGuiIO->DisplaySize;
    displaySize.x = (displaySize.x / 2.0f) - windowSize.x / 2.0f;
    displaySize.y = (displaySize.y / 2.0f) - windowSize.y / 2.0f;

    ImGui::SetNextWindowPos(displaySize);
    ImGui::SetNextWindowSize(windowSize);
    ImGui::Begin("GameRender...", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove| ImGuiWindowFlags_NoScrollWithMouse);
    ImGui::Text("Carregando UI");

    ImGui::ProgressBar(static_cast<float>(current) / static_cast<float>(max), ImVec2(windowSize.x, 16));

    ImGui::Text("%d/%d", max, current);

    ImGui::End();

    return current >= max;
}

void Editor::PostLoadGameObject(GameObject *gameObject) {
    if (!m_mainCamera || !gameObject) {
        return;
    }
    
    if (Scene::HasScene())
        Scene::GetCurrentScene()->insert(gameObject);
    
    gameObject->initialize(false);

    s_objectSelected = gameObject;
}

void Editor::SetupTextureMaterial(Material *material, int id, grr::gTexture *texture) {
    material->setTexture(id, texture);
}

std::string GenerateScriptHeader(const std::string& name) {
    auto source = File::GetFileContent(File::GetFile("/example/script.hpp", true));
    if (source.empty()) {
        Console::LogError("[ SCRIPT ] falied to generate source file.");
        return "";
    }
    source = String::replace_all(source, "#name", name);
    return source;
}

std::string GenerateScriptSource(const std::string& name) {
    auto source = File::GetFileContent(File::GetFile("/example/script.cpp", true));
    if (source.empty()) {
        Console::LogError("[ SCRIPT ] falied to generate source file.");
        return "";
    }
    source = String::replace_all(source, "#name", name);
    return source;
}

void Editor::CreateScriptTemplate(const std::string &path, const std::string &name) {
    auto script_hpp = GenerateScriptHeader(name);
    if (script_hpp.empty()) {
        return;
    }

    auto script_cpp = GenerateScriptSource(name);
    if (script_cpp.empty()) {
        return;
    }

    // write .hpp

    {
        std::ofstream os_header(path  + "/" + name + ".hpp");
        os_header << script_hpp << std::endl;
        os_header.close();
    }

    // write .cpp
    {
        std::ofstream os_source(path + "/" + name + ".cpp");
        os_source << script_cpp << std::endl;
        os_source.close();
    }

    Command::exec("code '" + path  + "/" + name + ".hpp" + "'");
}

// PopupMenu
void Editor::PopupMenuSettings() {
    ImVec2 vViewportWorkSize = m_viewport->WorkSize;
    ImVec2 vPopupMenuSize(300, 400);

    ImGui::SetNextWindowSize(vPopupMenuSize);
    ImGui::SetNextWindowPos((vViewportWorkSize / 2.0f) - (vPopupMenuSize / 2.0f));
    auto isOpen = ImGui::BeginPopup("##popup_menu_settings");
    if (!isOpen) {
        return;
    }

    auto title = "Settings";

    auto textSize = ImGui::CalcTextSize(title);

    ImGui::SetCursorPosX((vPopupMenuSize.x / 2.0f) - (textSize.x / 2.0f));
    ImGui::Text("%s", title);

    ImGui::NewLine();

    RenderSettings();

    ImGui::EndPopup();
}

void Editor::PopupRenameMaterial() {
    auto isOpen = ImGui::BeginPopupModal("##popup_rename_material");
    if (!isOpen || !m_materialToRename) {
        return;
    }

    ImGui::Text("Rename");
    if (!ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked(0))
        ImGui::SetKeyboardFocusHere(0);

    auto name = m_materialToRename->getName();
    if (name.size() < 256) {
        name.resize(256, '\0');
    }

    if (ImGui::InputText("##popup_rename_file", name.data(), name.size(), ImGuiInputTextFlags_EnterReturnsTrue)) {
        name.erase(std::remove(name.begin(), name.end(), '\0'), name.end());

        m_materialToRename->setName(name);

        m_materialToRename = nullptr;

        ImGui::CloseCurrentPopup();
    }

    ImGui::EndPopup();
}

void Editor::PopupCreateMaterial() {
}

void Editor::PopupRenameFile() {
    auto isOpen = ImGui::BeginPopup("##popup_rename_file");
    if (!isOpen) {
        return;
    }

    ImGui::Text("Rename");
    if (!ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked(0))
        ImGui::SetKeyboardFocusHere(0);

    auto path = m_SelectedPath.string();

    auto name = m_SelectedPath.filename().string();
    if (name.size() < 256) {
        name.resize(256, '\0');
    }

    if (ImGui::InputText("##popup_rename_file", name.data(), name.size(), ImGuiInputTextFlags_EnterReturnsTrue)) {
        name.erase(std::remove(name.begin(), name.end(), '\0'), name.end());

        File::rename(path, m_SelectedPath.replace_filename(name));

        ImGui::CloseCurrentPopup();
    }

    ImGui::EndPopup();
}

void Editor::BuildProject() {
    Console::LogInfo("[ Build ] Initialize");

    auto path = File::GetFile("/.build");
    if (!File::Exists(path)) {
        return Console::LogError("[ Build ] Falied to build project, internal data not found.");
    }

    std::function<void()> func = std::bind(&Editor::PostBuildScriptFromProject, this);

    if (func)
        Threadpool::AddTask(&Editor::CompileScript, this, path, func);
}

void Editor::PostBuildScriptFromProject() {

    Threadpool::AddTask([] {
        std::vector<std::string> cmd = {
            "/home/grsource/Documentos/gr-project/gr-engine-linux-project/bin/gr-engine-linux-project " +
                File::GetFile("")
        };
        
        Command::exec(cmd, [](std::string line) {
            Console::LogWarning(line);
        });
    });
}

void Editor::ReconfigScript() {
    Console::LogInfo("[ BUILD ] Initialized reconfigation...");

    auto component = File::GetFile("/component", true);
    if (!File::Exists(component)) {
        return Console::LogError("[ BULD ] Falied to load internal data component.");
    }

    auto path = File::GetFile("/.build");
    if (File::Exists(path)) {
        File::Delete(path);
    }

    File::CreateDirectory(path);

    if (!File::Copy(component, path)) {
        return Console::LogError("[ BULD ]" + File::GetError());
    }

    Console::LogInfo("[ BUILD ] building scipt...");

    PlayGame();
}

void Editor::BuildScript() {
    Console::LogInfo("[ Build ] Initialize");

    auto path = File::GetFile("/.build");
    if (!File::Exists(path)) {
        return Console::LogError("[ Build ] Falied to build project, internal data not found.");
    }

    CompileScript(path, std::bind(&Editor::LoadScript, this));
}

void Editor::CompileScript(std::string path, std::function<void()> func) {
    auto build = File::GetFile("/script/build.sh", true);
    if (!File::Exists(build)) {
        return Console::LogError("Build script do not found.");
    }

    auto project_path = File::GetFile("");
    if (!File::Exists(project_path)) {
        return Console::LogError("Project do not found.");
    }

    std::vector<std::string> command {
        "chmod +x " + build,
        "sh " + build + " '" + project_path + "'"
    };

    if (!Command::exec(command, [](std::string line) {
            Console::LogWarning(line);
        })) {
            Console::LogError("[ Editor ] Falied to execute build.");
        }

    if (func)
        Event::Add(func);
}

void Editor::SaveScene(const std::string &name, Scene *scene) {
    Scene::Use(scene);

    scene->save(File::GetAsset("/scene/") + name + ".scn");
}

void Editor::setActiveCamera(bool value) {
    if (value && tempCam) {
        m_cameraObject = tempCam;
        m_mainCamera = tempCam->hasComponent<Camera>() ? tempCam->getComponent<Camera>() : tempCam->addComponent<Camera>();

        if (Camera::SetActive) {
            Camera::SetActive(m_mainCamera);
        }
        return;
    }

    tempCam = m_cameraObject;

    m_cameraObject = nullptr;
    m_mainCamera = nullptr;
}

void Editor::showShaderListPopup() {
    auto bOpen = ImGui::BeginPopup("##popup_menu_shader_list");
    if (!bOpen) { return; }

    renderShaderList();

    ImGui::EndPopup();
}

void Editor::renderShaderList() {
    auto &shader_list = AssetsManager::GetShaderList();

    for (auto &&[name, shader] : shader_list) {
        ImGui::Text("%s", name.c_str());

        ImGui::SameLine();

        if (ImGui::Button("Reload")) {
            //
        }
    }
}

void Editor::showMeshListPopup() {
    auto bOpen = ImGui::BeginPopup("##popup_menu_mesh_list");
    if (!bOpen) { return; }

    renderMeshList();

    ImGui::EndPopup();
}

void Editor::renderMeshList() {
    ImGui::SeparatorText("Mesh List");

    auto &mesh_list = AssetsManager::GetMeshListMap();
    if (mesh_list.empty()) { return; }

    for (auto &&[name, mesh] : mesh_list) {
        ImGui::Text("%s", name.c_str());

        auto vertices = mesh->getVertices();
        ImGui::Text("Vertices: %ld", vertices.size());
        
        auto indices = mesh->getIndices();
        ImGui::Text("Indices: %ld", indices.size());

        auto bones = mesh->getBoneWeight();
        ImGui::Text("Bone: %ld", bones.size());

        ImGui::Separator();
    }
}

GameObject *Editor::CreatePrimitive(int type) {
    auto go = GameObject::Create("Cube");

    auto mesh = AssetsManager::FindMesh("Cube");
    if (!mesh) {
        mesh = AssetsManager::CreateMesh("Cube");

        std::vector<Vertex3D> vertices = {
            Vertex3D{Vector3{-1.000000, -1.000000,  1.000000}, Vector3{-1.000000, -0.000000, -0.000000}, Vector2{0.625000, 0.000000}, 0},
            Vertex3D{Vector3{-1.000000,  1.000000,  1.000000}, Vector3{-0.000000, -0.000000, -1.000000}, Vector2{0.375000, 0.000000}, 0},
            Vertex3D{Vector3{-1.000000, -1.000000, -1.000000}, Vector3{ 1.000000, -0.000000, -0.000000}, Vector2{0.375000, 0.000000}, 0},

            Vertex3D{Vector3{-1.000000, -1.000000,  1.000000}, Vector3{-1.000000, -0.000000, -0.000000}, Vector2{0.625000, 0.000000}, 0},
            Vertex3D{Vector3{-1.000000,  1.000000,  1.000000}, Vector3{-0.000000, -0.000000, -1.000000}, Vector2{0.375000, 0.000000}, 0},
            Vertex3D{Vector3{-1.000000, -1.000000, -1.000000}, Vector3{ 1.000000, -0.000000, -0.000000}, Vector2{0.375000, 0.000000}, 0},

            Vertex3D{Vector3{-1.000000, -1.000000,  1.000000}, Vector3{-1.000000, -0.000000, -0.000000}, Vector2{0.625000, 0.000000}, 0},
            Vertex3D{Vector3{-1.000000,  1.000000,  1.000000}, Vector3{-0.000000, -0.000000, -1.000000}, Vector2{0.375000, 0.000000}, 0},
            Vertex3D{Vector3{-1.000000, -1.000000, -1.000000}, Vector3{ 1.000000, -0.000000, -0.000000}, Vector2{0.375000, 0.000000}, 0},

            Vertex3D{Vector3{-1.000000, -1.000000,  1.000000}, Vector3{-1.000000, -0.000000, -0.000000}, Vector2{0.625000, 0.000000}, 0},
            Vertex3D{Vector3{-1.000000,  1.000000,  1.000000}, Vector3{-0.000000, -0.000000, -1.000000}, Vector2{0.375000, 0.000000}, 0},
            Vertex3D{Vector3{-1.000000, -1.000000, -1.000000}, Vector3{ 1.000000, -0.000000, -0.000000}, Vector2{0.375000, 0.000000}, 0},

            Vertex3D{Vector3{-1.000000, -1.000000,  1.000000}, Vector3{-1.000000, -0.000000, -0.000000}, Vector2{0.625000, 0.000000}, 0},
            Vertex3D{Vector3{-1.000000 , 1.000000,  1.000000}, Vector3{-0.000000, -0.000000, -1.000000}, Vector2{0.375000, 0.000000}, 0},
            Vertex3D{Vector3{-1.000000, -1.000000, -1.000000}, Vector3{ 1.000000, -0.000000, -0.000000}, Vector2{0.375000, 0.000000}, 0},

            Vertex3D{Vector3{-1.000000, -1.000000,  1.000000}, Vector3{-1.000000, -0.000000, -0.000000}, Vector2{0.625000, 0.000000}, 0},
            Vertex3D{Vector3{-1.000000,  1.000000,  1.000000}, Vector3{-0.000000, -0.000000, -1.000000}, Vector2{0.375000, 0.000000}, 0},
            Vertex3D{Vector3{-1.000000, -1.000000, -1.000000}, Vector3{ 1.000000, -0.000000, -0.000000}, Vector2{0.375000, 0.000000}, 0},

            Vertex3D{Vector3{-1.000000, -1.000000,  1.000000}, Vector3{-1.000000, -0.000000, -0.000000}, Vector2{0.625000, 0.000000}, 0},
            Vertex3D{Vector3{-1.000000,  1.000000,  1.000000}, Vector3{-0.000000, -0.000000, -1.000000}, Vector2{0.375000, 0.000000}, 0},
            Vertex3D{Vector3{-1.000000, -1.000000, -1.000000}, Vector3{ 1.000000, -0.000000, -0.000000}, Vector2{0.375000, 0.000000}, 0},

            Vertex3D{Vector3{-1.000000, -1.000000,  1.000000}, Vector3{-1.000000, -0.000000, -0.000000}, Vector2{0.625000, 0.000000}, 0},
            Vertex3D{Vector3{-1.000000,  1.000000,  1.000000}, Vector3{-0.000000, -0.000000, -1.000000}, Vector2{0.375000, 0.000000}, 0},
            Vertex3D{Vector3{-1.000000, -1.000000, -1.000000}, Vector3{ 1.000000, -0.000000, -0.000000}, Vector2{0.375000, 0.000000}, 0},

            Vertex3D{Vector3{-1.000000, -1.000000,  1.000000}, Vector3{-1.000000, -0.000000, -0.000000}, Vector2{0.625000, 0.000000}, 0},
            Vertex3D{Vector3{-1.000000,  1.000000,  1.000000}, Vector3{-0.000000, -0.000000, -1.000000}, Vector2{0.375000, 0.000000}, 0},
            Vertex3D{Vector3{-1.000000, -1.000000, -1.000000}, Vector3{ 1.000000, -0.000000, -0.000000}, Vector2{0.375000, 0.000000}, 0},

            Vertex3D{Vector3{-1.000000, -1.000000,  1.000000}, Vector3{-1.000000, -0.000000, -0.000000}, Vector2{0.625000, 0.000000}, 0},
            Vertex3D{Vector3{-1.000000,  1.000000,  1.000000}, Vector3{-0.000000, -0.000000, -1.000000}, Vector2{0.375000, 0.000000}, 0},
            Vertex3D{Vector3{-1.000000, -1.000000, -1.000000}, Vector3{ 1.000000, -0.000000, -0.000000}, Vector2{0.375000, 0.000000}, 0},

            Vertex3D{Vector3{-1.000000, -1.000000,  1.000000}, Vector3{-1.000000, -0.000000, -0.000000}, Vector2{0.625000, 0.000000}, 0},
            Vertex3D{Vector3{-1.000000,  1.000000,  1.000000}, Vector3{-0.000000, -0.000000, -1.000000}, Vector2{0.375000, 0.000000}, 0},
            Vertex3D{Vector3{-1.000000, -1.000000, -1.000000}, Vector3{ 1.000000, -0.000000, -0.000000}, Vector2{0.375000, 0.000000}, 0},

            Vertex3D{Vector3{-1.000000, -1.000000,  1.000000}, Vector3{-1.000000, -0.000000, -0.000000}, Vector2{0.625000, 0.000000}, 0},
            Vertex3D{Vector3{-1.000000,  1.000000,  1.000000}, Vector3{-0.000000, -0.000000, -1.000000}, Vector2{0.375000, 0.000000}, 0},
            Vertex3D{Vector3{-1.000000, -1.000000, -1.000000}, Vector3{ 1.000000, -0.000000, -0.000000}, Vector2{0.375000, 0.000000}, 0}

        };

        for (auto &v : vertices) {
            mesh->addVertex(v);
        }
    }


    auto mf = go->addComponent<MeshFilter>();
    mf->Init();
    mf->setMesh(mesh);

    auto mr = go->addComponent<MeshRender>();
    mr->Init();

    mr->addMaterial();
    // mr->build();

    return GameObject::Instantiate(go);
}

GameObject *Editor::CreateEditorCamera() {
    auto gameObject = GameObject::Create("MainEngineCamera", "Engine");
    if (!gameObject) {
        return nullptr;
    }

    auto camera = gameObject->addComponent<Camera>();
    if (!camera) {
        return nullptr;
    }

    gameObject->initialize(false);
    return gameObject;
}



