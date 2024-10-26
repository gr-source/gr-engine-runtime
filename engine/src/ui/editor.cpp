#include "editor.hpp"

#include "postProcessing.hpp"
#include "editorController.h"
#include "renderManager.h"
#include "screenRender.hpp"
#include "console.hpp"
#include "renderer.h"
#include "settings.h"
#include "screen.h"
#include "engine.h"
#include "memory.h"
#include "skybox.h"
#include "timer.h"


#include "core/filmbox/filmbox.hpp"

#include "core/binary/gModel3D.hpp"

#include "core/cmd/cmd.hpp"

#include "core/file/initialization.hpp"
#include "core/file/config.hpp"
#include "core/file/file.hpp"

#include "core/assetsManager.hpp"
#include "core/threadpool.hpp"
#include "core/statistics.hpp"
#include "core/scene.hpp"
#include "core/event.hpp"

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
#include "ui/View.h"

#include "scriptManager/scriptManager.hpp"

#include "geometry/batching.hpp"

#include "ecs/manager.hpp"

#include <linux/input-event-codes.h>
#include <filesystem>
#include <sstream>
#include <iomanip>

#define IMGUI_PAYLOAD_GAMEOBJECT "##payload_game_object"
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

std::unordered_map<IconType, grr::gTexture *> iconsUI;

ImGuiViewport *Editor::m_viewport = nullptr;

grr::gTexture *Editor::s_texturePlayGame = nullptr;

GameObject *Editor::s_objectSelected = nullptr;

ImGuiIO* Editor::s_io = nullptr;

bool Editor::bFileDoubleClick = false;

bool Editor::bCutAllFile = false;

bool Editor::bCopyAllFile = false;

bool Editor::bViewPortFocused = false;

bool Editor::bOpenPopupInfoDir = false;

bool Editor::bScriptReloaded = false;

bool Editor::bOpenPopupOptionGameObject = false;

bool Editor::pPopupFileManager = false;

bool Editor::bBuildDock = false;

ImVec4 Editor::FPSColor = {255, 255, 255, 255};

ImVec2 Editor::m_lastViewportSize = {0.0f, 0.0f};

ImGuizmo::OPERATION Editor::m_guizmoOperation = ImGuizmo::TRANSLATE;

ImGuizmo::MODE Editor::m_guizmoManipulate = ImGuizmo::WORLD;

grm::u32 Editor::s_maxProgress = 0;

grm::u8 Editor::m_currentPage = 0;

std::unordered_map<std::filesystem::path, bool> Editor::fLisDirSelected;

std::filesystem::path Editor::m_currentPath;

std::filesystem::path Editor::m_SelectedPath;

std::shared_ptr<Config> Editor::m_config = nullptr;

void Editor::Initialize() {
    m_viewport = ImGui::GetMainViewport();

    s_io = &ImGui::GetIO();

    m_currentPath = File::GetAsset("");

    m_config = std::make_shared<Config>(File::GetFile("/config.ini"));
    try {
        m_config->load();
    } catch (const std::runtime_error &e) {
        LOG_WARNING(e.what());
    }

    bBuildDock = File::Exists("imgui.ini");

    LoadReources();

    try {
        ScriptManager::Initialize(File::GetFile("/.build/component/lib/libgr-component.so"));
    } catch (const std::exception &e) {
        LOG_ERROR("Fatal error in script: " + std::string(e.what()));
    }

    auto lastCategory = m_config->getCategory();

    m_config->setCategory("scene");
    if (m_config->hasKey("main") && !Scene::HasScene()) {
        Scene::LoadScene(File::GetAsset("scene/" + m_config->getValue<std::string>("main") + ".scn"));
    }

    m_config->setCategory(lastCategory);

    Scene::Initialize();

    RenderManager::Initialize();

    ScreenRender::Initialize();

    Skybox::Initialize();

    LOG_WARNING_ARGS("InitializedMain: %d - %f", 34, 0.5f);
}

void Editor::Shutdown() {
    m_config->setCategory("scene");
    if (Scene::HasScene()) {
        m_config->setValue<std::string>("main", Scene::GetCurrentScene()->getName());
    }

    if (m_config) {
        m_config->save();
        m_config = nullptr;
    }

    Scene::Shutdown();

    RenderManager::Shutdown();

    Skybox::Shutdown();

    ScreenRender::Shutdown();

    ScriptManager::Shutdown();

    Console::Shutdown();

    iconsUI.clear();

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
    gModel3D::ImportFBX(path);
}

void Editor::ImportImage(const std::string &path) {
    auto assets_texture = File::GetAsset("texture");
    if (!File::Exists(assets_texture)) {
        return LOG_ERROR("[ Import ] Assets data project do not loaded.");
    }
    LOG_INFO("[ Import ] Importing texture...");

    if (!File::Copy(path, assets_texture)) {
        return LOG_ERROR("[ Import ] " + File::GetError());
    }
    LOG_INFO("[ Import ] Texture imported.");
}

void Editor::KeyBoardUp(grm::u32 key) {
    if (!(bViewPortFocused && key == KEY_ESC)) {
        return;
    }

    if (s_io)
        s_io->MouseDrawCursor = !s_io->MouseDrawCursor;
}

void Editor::LoadScript() {
    if (ScriptManager::IsValid()) {
        UnloadScript();
    }

    if (ScriptManager::CheckScriptLoaded("libgr-component.so")) {
        return LOG_ERROR("[ Script ] Fatal error data not unloaded.");
    }

    try {
        ScriptManager::Initialize(File::GetFile("/.build/component/lib/libgr-component.so"));

        Scene::ReloadObjects();
    } catch (const std::runtime_error &e) {
        LOG_ERROR(e.what());
    }

    if (ScriptManager::SetScreenSize) {
        ScriptManager::SetScreenSize(Settings::ViewportSize);
    }

    RenderManager::Initialize();

    ScreenRender::Initialize();

    Settings::bGameRunning = true;
}

void Editor::UnloadScript() {
    RenderManager::Shutdown();

    ScreenRender::Shutdown();

    ScriptManager::Shutdown();
}

// ===================================== END =========================================================
void Editor::RenderUI() {
    if (!Project::GetCurrent()) {
        return;
    }

    RenderDocking();

    RenderViewportUI();

    RenderScenesUI();

    RenderOtherUI();

    RenderConsoleWindow();

    RenderPropertiUI();

    if (Camera::Current && s_io->KeyCtrl && ImGui::IsKeyReleased(ImGuiKey_S)) {
        for (auto &&[name, scene] : Scene::GetScenes()) {
            Event::Add(&Editor::SaveScene, name, &(*scene));
        }
    }

    PopupComponentMenu();

    PopupMenuSettings();
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

    auto bPopupMenuSettings = false;

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
        ImGui::EndMenuBar();
    }

    ImGuiID dockspace_id = ImGui::GetID("DockSpace");
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
    ImGui::End();

    if (bPopupMenuSettings) {
        ImGui::OpenPopup("##popup_menu_settings");
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
                    
                    Scene::GetCurrentScene()->Add(target);
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
    
    if (ImGui::IsKeyReleased(ImGuiKey_F2)) {
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
    
    if (Manager::IsAvailable())
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
                    LOG_ERROR("GameObject do not found: " + name);
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
    const char* item_name[1]{"Empty GameObject"};
    auto id = 0;

    ImGui::Text("Add Object");
    if (ImGui::ListBox("##ListBoxOptionsGame", &id, item_name, 1)) {
        if (id == 0) {
            s_objectSelected = &Scene::GetCurrentScene()->CreateGameObject("Empty");
        }
        ImGui::CloseCurrentPopup();
    }

    ImGui::EndPopup();
}

void Editor::PopupOptionsGameObject() {
    if (!s_objectSelected || !Camera::Current) {
        return;
    }

    auto cam = Camera::Current();
    
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

    if (isOpenRename) {
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

void Editor::RenderScene() {
    auto cursorY = ImGui::GetCursorPosY();
    cursorY += 5.0f;

    ImVec2 curSize = ImGui::GetContentRegionAvail();
    if (Camera::Current && (m_lastViewportSize.x != curSize.x || m_lastViewportSize.y != curSize.y && curSize.x > 5.0f && curSize.y > 5.0f || bScriptReloaded)) {
        auto &viewport = Settings::ViewportSize = {static_cast<int>(curSize.x), static_cast<int>(curSize.y)};

        if (ScriptManager::SetScreenSize) {
            ScriptManager::SetScreenSize(Settings::ViewportSize);
        }

        auto camera = Camera::Current();
        if (camera) {
            camera->setAspectRatio(static_cast<float>(viewport.x) / static_cast<float>(viewport.y));
        }

        PostProcessing::Resize(Engine::MainPostProcess, viewport);
        
        m_lastViewportSize = curSize;
        bScriptReloaded = false;
    }

    auto texture = PostProcessing::GetTextureColor(Engine::ImGuiPostProcess);
    if (texture) {
        ImGui::Image(reinterpret_cast<ImTextureID>(texture->getID()), curSize, ImVec2(0, 1), ImVec2(1, 0));

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
        
        ImGui::SetCursorPos({curSize.x / 2 - cursorPos.x, cursorY});

        if (ImGui::ImageButton(reinterpret_cast<ImTextureID>(s_texturePlayGame->getID()), ImVec2(16, 16))) {
            if (!Settings::bGameRunning) {
                PlayGame();
            } else {
                StopGame();
            }
        }
    }

    ImGuizmo::SetDrawlist();
    ImGuizmo::SetRect(pos.x, pos.y + (cursorY - 5.0f), curSize.x, curSize.y);

    if (Camera::Current && s_objectSelected && s_objectSelected->hasComponent<Transform>()) {
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

        auto curtorPos = ImVec2(curSize.x - textSize.x, cursorY + textSize.y);

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

    if (s_objectSelected && !s_io->MouseDrawCursor && ImGui::IsWindowFocused() && s_io->KeyShift && ImGui::IsKeyReleased(ImGuiKey_D)) {
        s_objectSelected = &GameObject::Instantiate(s_objectSelected);
    }
}

void Editor::PlayGame() {
    s_objectSelected = nullptr;

    Scene::DestroyObjects();

    Threadpool::AddTask(&Editor::BuildScript);

    Console::log(LogInfo::DEFAULT, "Game playing.");
}

// Falta recarregar os scripts anteriores.
void Editor::StopGame() {
    Settings::bGameRunning = false;

    Scene::ReloadObjects();

    Console::log(LogInfo::DEFAULT, "Game stoped.");
}

void Editor::RenderGuizmo() {
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
    
    auto transform = s_objectSelected->getTransform();

    auto globalMatrix = transform->getGlobalMatrix();

    auto camera = Camera::Current();

    if (camera && ImGuizmo::Manipulate(camera->getViewmatrix().getData(), camera->getPerspesctive().getData(), m_guizmoOperation, m_guizmoManipulate, globalMatrix.getData())) {
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

    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(IMGUI_PAYLOAD_FILE)) {
        std::string path((char*)payload->Data, payload->DataSize);
        if (!path.empty() && std::filesystem::path(path).filename().extension() == ".gmd") {
            AssetsManager::LoadGameObject(path, &Editor::PostLoadGameObject);
        }
    }
    ImGui::EndDragDropTarget();
}

void Editor::PopupMenuCreateScene() {
    auto isOpen = ImGui::BeginPopup("##popup_menu_create_scene");
    if (!isOpen) {
        return;
    }

    ImGui::Text("Input scene name: ");

    if (!ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked(0))
        ImGui::SetKeyboardFocusHere(0);

    char name[256] = {0};
    if (ImGui::InputText("##input_scene_name", name, 256, ImGuiInputTextFlags_EnterReturnsTrue)) {
        std::string to_str = name;
        if (!to_str.empty()) {
            Scene::Create(name);
        }
        
        ImGui::CloseCurrentPopup();
    }

    ImGui::EndPopup();
}

void Editor::RenderFilesUI() {
    for (auto &entry : std::filesystem::directory_iterator(m_currentPath)) {
        auto path = entry.path();
        
        if (path.filename().string()[0] == '.') {
            continue;
        }
        RenderTreeNodeFile(path);
    }

    if (pPopupFileManager) {
        ImGui::OpenPopup("##popup_dir_manager");
        pPopupFileManager = false;
    }

    PopupDirmanager();

    PopupCreateDirectory();

    PopupCreateScript();
}

void Editor::RenderTreeNodeFile(std::filesystem::path path) {
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
    } else if (s_io->KeyShift && ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
        fLisDirSelected.emplace(path, true);
    } else if (ImGui::IsItemClicked(ImGuiMouseButton_Left) && fLisDirSelected.find(path) == fLisDirSelected.end()) {
        fLisDirSelected.clear();
    } else if (ImGui::IsItemClicked(ImGuiMouseButton_Left) && fLisDirSelected.find(path) != fLisDirSelected.end()) {
        fLisDirSelected.clear();
        fLisDirSelected.emplace(path, true);
    }

    if (!isOpen) {
        return;
    }

    if (!bDirectory && ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceNoDisableHover | ImGuiDragDropFlags_SourceNoHoldToOpenOthers)) {
        auto str = path.string();

        ImGui::Text("%s", str.c_str());
        ImGui::SetDragDropPayload(IMGUI_PAYLOAD_FILE, str.data(), str.size());
        ImGui::EndDragDropSource();
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

    std::array<const char *, 4> item_list{"Delete", "Rename", "Create directory", "Create script"};

    if (ImGui::ListBox("##DirManager", &idx, item_list.data(), item_list.size())) {
        switch (idx) {
        case 0:
            for (auto &&[path, isSeleceted] : fLisDirSelected) {
                File::Delete(path);
            }
            break;
        case 1:
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

    std::for_each(logs.begin(), logs.end(), &Editor::RenderLog);
    
    if (Console::bChanged) {
        ImGui::SetScrollHereY(1.0f);
        Console::bChanged = false;
    }
}

void Editor::RenderLog(LogInfo &info) {
    ImVec4 color = {1.0f, 1.0f, 1.0f, 1.0f};
    switch (info.type) {
    case LogInfo::LogType::INFO:
        color = {0.0f, 1.0f, 0.0f, 1.0f};
        break;
    case LogInfo::LogType::WARNING:
        color = {1.0f, 1.0f, 0.0f, 1.0f};
        break;
    case LogInfo::LogType::ERROR:
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

    ImGui::SeparatorText("Editor");

    std::array<const char *, 3> editor_name{"None", "Visual Studio Code", "Sublime Text"};

    m_config->setCategory("config");

    int item = m_config->hasKey("use_editor") ? m_config->getValue<int>("use_editor") : m_config->setValue<int>("use_editor", 0);
    if (ImGui::Combo("##render_settings_list_box", &item, editor_name.data(), editor_name.size())) {
        m_config->setValue<int>("use_editor", item);

        switch (item) {
        case 1: // Visual Studio Code
            return ConfigureVSCode();
        }
        ImGui::CloseCurrentPopup();
    }

    ImGui::SeparatorText("World");

    auto bFpsLimited = m_config->hasKey("fps_limit") ? m_config->getValue<bool>("fps_limit") : m_config->setValue<bool>("fps_limit", true);
    if (ImGui::Checkbox("Fps Limited", &bFpsLimited)) {
        m_config->setValue<bool>("fps_limit", bFpsLimited);
    }
}

// TODO: adicionar suporte ao vs code
void Editor::ConfigureVSCode() {
    LOG_INFO("Initializing Visual Studio Code Setup...");
    
    auto internal_vscode = File::GetFile("/config/vscode/c_cpp_properties.json", true);
    if (!File::Exists(internal_vscode)) {
        return LOG_ERROR("Falied to load internal data config.");
    }

    auto header = File::GetFile("/.build/component/include");
    if (!File::Exists(header)) {
        return LOG_ERROR("Project do not configured.");
    }

    auto source = File::GetFileContent(internal_vscode);
    if (source.empty()) {
        return LOG_ERROR("Falied to read internal data.");
    }

    source = String::replace_all(source, "${header_internal_script}", header);

    auto dest_vscode = File::GetAsset(".vscode");
    if (File::Exists(dest_vscode)) {
        File::Delete(dest_vscode);
    }

    if (!File::CreateDirectory(dest_vscode)) {
        return LOG_ERROR("Falied to create config in project.");
    }

    std::ofstream os(File::GetAsset(".vscode/c_cpp_properties.json"));
    os << source << std::endl;
    os.close();

    LOG_WARNING("Visual Studio Code has been configured.");
}

void Editor::RenderPropertiUI() {
    ImGui::Begin("Propriedades", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

    if (ImGui::BeginTabBar("##propertimenubar")) {
        RenderGameObjectPropertiUI();
        
        if (ImGui::BeginTabItem("Estatística", nullptr)) {
            RenderStatisticsMenu();
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
    ImGui::End();
}

void Editor::RenderStatisticsMenu() {
    ImGui::SeparatorText("Scripts");
            
    ImGui::Text("Entidades: %ld", Manager::GetEntityCount());
    ImGui::Text("GameObject: %ld", GameObject::GetGameObjectCount());

    // ImGui::Text("Memoria: %s", getfilesize(Memory::totalMemoryUsage).c_str());

    ImGui::SeparatorText("Cena");
    // ImGui::Text("Vertices: %d | %s", gameStatics.vertices, getfilesize(gameStatics.vertices * sizeof(Vertex3D)).c_str());
    // ImGui::Text("Indice: %d | %s", gameStatics.indices, getfilesize(gameStatics.indices * sizeof(grm::u32)).c_str());
    // ImGui::Text("Triangulos: %.2f", (gameStatics.indices / 3.0f));

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
    // ImGui::Text("Cameras: %ld",   Camera::Get.size());
    if (ScriptManager::GetListLight)
        ImGui::Text("Luzes: %ld",     ScriptManager::GetListLight().size());
    ImGui::Text("Draw Call Count: %ld", Statistics::m_drawCallCount);
    ImGui::Text("Draw Render: %ld/%ld", Statistics::m_totalRender, Statistics::m_drawRenderCount);
    ImGui::Text("Batched: %ld", Batching::GetBatchedCount());
}

void Editor::RenderGameObjectPropertiUI() {
    if (!s_objectSelected || !ScriptManager::GetComponentPool) {
        return;
    }

    auto entt = s_objectSelected->getEntity();

    auto isOpenPopup = false;
    if (ImGui::BeginTabItem("GameObject", nullptr)) {
        ImGui::Image(reinterpret_cast<ImTextureID>(iconsUI[IconType::ICON_GAMEOBJECT]->getID()), ImVec2(16, 16));

        ImGui::SameLine();

        ImGui::Text("[ %s - %s ]", s_objectSelected->getName().c_str(), s_objectSelected->getTag().c_str());

        ImGui::SameLine();

        auto isActive = s_objectSelected->isActive();
        if (ImGui::Checkbox("##check_box_is_active", &isActive)) {
            s_objectSelected->setActive(isActive);
        }


        auto &components = s_objectSelected->getEntity()->getComponents();
        for (auto &&[name, component] : components) {
            if (name == "Transform") {
                s_objectSelected->RenderProperty<Transform>(Editor::OnPropertyTransform);
                continue;
            }
            
            if (name == "Light") {
                if (s_objectSelected->RenderProperty<Light>(Editor::OnPropertyLight)) {
                    Event::Add(&Entity::remove, s_objectSelected->getEntity(), name);
                }
                continue;
            }

            if (name == "MeshRender") {
                if (s_objectSelected->RenderProperty<MeshRender>(Editor::OnPropertyMeshRender)) {
                    Event::Add(&Entity::remove, s_objectSelected->getEntity(), name);
                    Event::Add(&Entity::remove, s_objectSelected->getEntity(), "MeshFilter");
                }
                continue;
            }

            if (name == "MeshFilter") {
                if (s_objectSelected->RenderProperty<MeshFilter>(Editor::OnPropertyMeshFilter)) {
                    Event::Add(&Entity::remove, s_objectSelected->getEntity(), name);
                }
                continue;
            }

            if (name == "BoxCollider") {
                if (s_objectSelected->RenderProperty<BoxCollider>(Editor::OnPropertyBoxCollider)) {
                    Event::Add(&Entity::remove, s_objectSelected->getEntity(), name);
                }
                continue;
            }

            if (name == "Rigidbody") {
                if (s_objectSelected->RenderProperty<Rigidbody>(Editor::OnPropertyRigidbody)) {
                    Event::Add(&Entity::remove, s_objectSelected->getEntity(), name);
                }
                continue;
            }


            if (!RenderComponents(name, &(*component))) {
                Event::Add(&Entity::remove, s_objectSelected->getEntity(), name);
                continue;
            }
        }

        if (ImGui::SmallButton("AddComponent")) {
            isOpenPopup = true;
        }

        ImGui::EndTabItem();
    }

    if (isOpenPopup) {
        ImGui::OpenPopup("##popup_add_component");
    }
    PopupAddComponent();
}

bool Editor::RenderComponents(const std::string &name, Component *component) {
    auto &componentPool = ScriptManager::GetComponentPool();

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
        LOG_ERROR("Invalid component: " + name);
        return false;
    }
    
    auto &fieldList = it->second->getFields();

    for (auto &field : fieldList) {
        RenderField(field, component);
    }

    return true;
}

void Editor::RenderField(Field &field, Component *component) {
    auto name = field.getName();
    auto type = field.getType();

    if (type == "bool") {
        auto data = Field::GetFieldData<bool>(component, field);

        ImGui::PushStyleColor(ImGuiCol_Button, {0.0f, 0.0f, 0.0f, 1.0f});
        *data = ImGui::Button(name.c_str());
        ImGui::PopStyleColor();
        return;
    }

    ImGui::Text("%s:", name.c_str());
    name = "##" + name;
    
    if (type == "float") {
        auto data = Field::GetFieldData<float>(component, field);

        ImGui::DragFloat(name.c_str(), data);
    } else if (type == "int") {
        auto data = Field::GetFieldData<int>(component, field);

        ImGui::DragInt(name.c_str(), data);
    } else if (type == "bool") {
        auto data = Field::GetFieldData<bool>(component, field);

        ImGui::Checkbox(name.c_str(), data);
    } else if (type == "Vector3") {
        auto data = Field::GetFieldData<Vector3>(component, field);

        ImGui::DragFloat3(name.c_str(), (float *)data);
    }
}

void Editor::PopupAddComponent() {
    if (!ScriptManager::GetComponentPool && !s_objectSelected) {
        return;
    }
    
    auto isOpen = ImGui::BeginPopup("##popup_add_component");
    if (!isOpen) {
        return;
    }
    auto &componentPool = ScriptManager::GetComponentPool();

    std::vector<std::string> m_componentName;
    for (auto &&[name, pool] : componentPool) {
        m_componentName.push_back(name);
    }

    std::sort(m_componentName.begin(), m_componentName.end(), [](const auto &lhs, const auto &rhs) {
        return lhs < rhs;
    });

    for (auto &name : m_componentName) {
        if (ImGui::SmallButton(name.c_str())) {
            s_objectSelected->getEntity()->addComponent(name);

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

    const char* text[] {"X", "Y", "Z"};

    auto position = transform->getLocalPosition();
    ImGui::Text("Posição:");
    if (ImGui::DragFloatEx("##Posicao", text, position.data, 1.0f, 0.0f, 0.0f, "%.3f")) {
        transform->setLocalPosition(position);
    }

    auto scale = transform->getLocalScale();
    ImGui::Text("Escala:");
    if (ImGui::DragFloatEx("##Escala", text, scale.data, 1.0f, 1.0f, 0.0f, "%.3f")) {
        transform->setLocalScale(scale);
    }

    auto euler = Math::eulerAngles(transform->getLocalRotation());

    ImGui::Text("Rotação:");
    if (ImGui::DragFloatEx("##Rotacao", text, euler.data, 1.0f, -360.0f, 360.0f)) {
        transform->setLocalRotation(Math::normalize(Math::euler({Math::rad(euler.x), Math::rad(euler.y), Math::rad(euler.z)})));
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

    Vector2 scale;
    if (ImGui::DragFloat2("##meshFilter_uv_scale", reinterpret_cast<float *>(&scale))) {
        //
    }

    ImGui::TreePop();
}

bool Editor::OnPropertyMeshRender(MeshRender *meshRender) {
    auto bExists = true;

    auto isOpen = ImGui::CollapsingHeader("MeshRender", &bExists);
    if (!bExists) {
        return true;
    }
    
    if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
        ImGui::OpenPopup("##game_object_component_menu");
    }

    if (!isOpen) {
        return false;
    }

    auto elem = meshRender->getFlags();

    // int  = Batching::Batching_Flags_Dynamic;
        
    const char* elems_names[4] = {"Dynamic", "GroupMesh", "InstanceDynamic", "InstancedStatic"};
    
    const char* elem_name = (elem >= 0 && elem < 4) ? elems_names[elem] : "Unknown";

    ImGui::Text("Mesh Optimize:");
    if (ImGui::SliderInt("##mesh_optimize", &elem, 0, 4 - 1, elem_name)) {
        meshRender->setMeshOptimize(elem);

        // auto gameObject = meshRender->getGameObject();
        // auto &transform = gameObject->getTransform();

        // for (auto &child : transform.getChildren()) {
        //     auto obj = child->getGameObject();
        //     if (obj->hasComponent<MeshRender>()) {
        //         obj->get
        //     }
        // }
    }

    for (auto &material : meshRender->getMaterials()) {
        auto isOpen = ImGui::TreeNode(material->getName().c_str());
        if (!isOpen) {
            continue;
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
        ImGui::TreePop();
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
        LOG_ERROR("[ GUI ] Texture do not loaded, patg is empty.");
        return ImGui::EndDragDropTarget();
    }
    AssetsManager::LoadTexture(path, &Editor::SetupTextureMaterial, material, 0);
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

    ImGui::Text("Type:");
    if (ImGui::SliderInt("##LightType", &elem, 0, 3 - 1, elem_name)) {
        light->setType(elem);
    }
    
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

    auto bDynamic = rigidbody->getDinamic();
    if (ImGui::Checkbox("##rigidbody_dynamic", &bDynamic)) {
        rigidbody->setDynamic(bDynamic);
    }

    const char* text[] {"X", "Y", "Z"};

    auto freese = rigidbody->getLookRotation();
    ImGui::Text("Freeze");
    if (ImGui::DragFloatEx("##rigidbody_freeze", text, reinterpret_cast<float *>(&freese), 1.0f, 0.0f, 0.0f, "%.3f")) {
        rigidbody->setLookRotation(freese);
    }

    return false;
}

void Editor::OnPropertyAnimator(Animator *animator) {
    auto &m_current = !animator->m_animation ? animator->m_animation : animator->m_nextAnimation;

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

        if (ImGui::BeginDragDropTarget()) {
            ImGuiDragDropFlags target_flags = 0;
            // target_flags |= ImGuiDragDropFlags_AcceptBeforeDelivery;
            // target_flags |= ImGuiDragDropFlags_SourceAllowNullID;

            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(IMGUI_PAYLOAD_FILE, target_flags)) {
                std::string path((char*)payload->Data, payload->DataSize);
                if (!path.empty() && std::filesystem::path(path).filename().extension() == ".anm") {
                    animator->insert(AssetsManager::LoadAnimation(path));
                }
            }
            ImGui::EndDragDropTarget();
        }

        if (isOpen) {
            auto &animations = animator->m_animations;
            for (auto &&[name, animation] : animations) {
                ImGui::Selectable(name.c_str(), m_current == animation);
                if (ImGui::IsItemClicked()) {
                    m_current = animation;
                }
            }
            ImGui::TreePop();
        }
        ImGui::TreePop();
    }
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
    AssetsManager::LoadTexture(File::GetAsset("icon/icons8-file-30.png", true),                    &Editor::SetupTexture, IconType::ICON_FILE);
    AssetsManager::LoadTexture(File::GetAsset("icon/icons8-folder-50.png", true),                  &Editor::SetupTexture, IconType::ICON_FOLDER);
    AssetsManager::LoadTexture(File::GetAsset("icon/icons8-go-back-50.png", true),                 &Editor::SetupTexture, IconType::ICON_BACK_FILE);
    AssetsManager::LoadTexture(File::GetAsset("icon/icons8-3d-object-50.png", true),               &Editor::SetupTexture, IconType::ICON_GAMEOBJECT);
    AssetsManager::LoadTexture(File::GetAsset("icon/icons8-lápis-30.png", true),                   &Editor::SetupTexture, IconType::ICON_PENCIL);
    AssetsManager::LoadTexture(File::GetAsset("icon/icons8-lâmpada-globular-64.png", true),        &Editor::SetupTexture, IconType::ICON_LIGHT);
    AssetsManager::LoadTexture(File::GetAsset("icon/icons8-textura-de-renderização-32.png", true), &Editor::SetupTexture, IconType::ICON_TEXTURE);
    AssetsManager::LoadTexture(File::GetAsset("icon/icons8-png-24.png", true),                     &Editor::SetupTexture, IconType::ICON_PNG);
    AssetsManager::LoadTexture(File::GetAsset("icon/icons8-reproduzir-30.png", true),              &Editor::SetupTexture, IconType::ICON_PLAY_GAME);
    AssetsManager::LoadTexture(File::GetAsset("icon/icons8-parar-50.png", true),                   &Editor::SetupTexture, IconType::ICON_STOP_GAME);
    AssetsManager::LoadTexture(File::GetAsset("icon/icons8-resíduos-50.png", true),                &Editor::SetupTexture, IconType::ICON_BIN);
    AssetsManager::LoadTexture(File::GetAsset("icon/icons8-importar-50.png", true),                &Editor::SetupTexture, IconType::ICON_IMPORT);
    AssetsManager::LoadTexture(File::GetAsset("icon/icons8-ordenar-para-a-direita-50.png", true),  &Editor::SetupTexture, IconType::ICON_ASCENDENTE);
    AssetsManager::LoadTexture(File::GetAsset("icon/icons8-ordem-descendente-50.png", true),       &Editor::SetupTexture, IconType::ICON_DESCENDENTE);
    AssetsManager::LoadTexture(File::GetAsset("icon/icons8-editar-24.png", true),                  &Editor::SetupTexture, IconType::ICON_EDIT);
    AssetsManager::LoadTexture(File::GetAsset("icon/motion-graphics.png", true),                   &Editor::SetupTexture, IconType::ICON_ANIMATION);
    AssetsManager::LoadTexture(File::GetAsset("icon/shade.png", true),                             &Editor::SetupTexture, IconType::ICON_SHADER);
    AssetsManager::LoadTexture(File::GetAsset("icon/studio-set.png", true),                        &Editor::SetupTexture, IconType::ICON_SCENE);
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

bool Editor::ProgressLoad(int max, int current) {
    const ImVec2 windowSize(300, 200);

    ImVec2 displaySize = s_io->DisplaySize;
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
    if (!Camera::Current || !gameObject) {
        return;
    }
    
    auto camera = Camera::Current();
    if (!camera) {
        return;
    }
    
    // auto camTransform = camera->getTransform();

    // auto transform = gameObject->getTransform();
    // transform->setLocalPosition(camera->getPosition() + (camTransform->getforward() * 5.5f));

    s_objectSelected = gameObject;
}

void Editor::SetupTextureMaterial(Material *material, int id, grr::gTexture *texture) {
    material->setTexture(id, texture);
}

std::string GenerateScriptHeader(const std::string& name) {
    auto source = File::GetFileContent(File::GetFile("/example/script.hpp", true));
    if (source.empty()) {
        LOG_ERROR("[ SCRIPT ] falied to generate source file.");
        return "";
    }
    source = String::replace_all(source, "#name", name);
    return source;
}

std::string GenerateScriptSource(const std::string& name) {
    auto source = File::GetFileContent(File::GetFile("/example/script.cpp", true));
    if (source.empty()) {
        LOG_ERROR("[ SCRIPT ] falied to generate source file.");
        return "";
    }
    source = String::replace_all(source, "#name", name);
    return source;
}

#include <regex>
void Editor::CreateScriptTemplate(const std::string &path, const std::string &name) {
    std::ofstream os_header(path  + "/" + name + ".hpp");
    os_header << GenerateScriptHeader(name);
    os_header.close();

    std::ofstream os_source(path + "/" + name + ".cpp");
    os_source << GenerateScriptSource(name);
    os_source.close();

    std::string p = m_currentPath.string();
    p = std::regex_replace(p, std::regex(" "), "\\ ");

    Command::exec("code " + p);
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

    const char *title = "Settings";

    auto textSize = ImGui::CalcTextSize(title);

    ImGui::SetCursorPosX((vPopupMenuSize.x / 2.0f) - (textSize.x / 2.0f));
    ImGui::Text("%s", title);

    ImGui::NewLine();

    RenderSettings();

    ImGui::EndPopup();
}

void Editor::PopupRenameFile() {
    auto isOpen = ImGui::BeginPopup("##popup_rename_file");
    if (!isOpen) {
        return;
    }

    auto name = m_SelectedPath.filename().string();

    ImGui::EndPopup();
}

void Editor::BuildProject() {
    LOG_INFO("[ Build ] Initialize");

    auto path = File::GetFile("/.build");
    if (!File::Exists(path)) {
        return LOG_ERROR("[ Build ] Falied to build project, internal data not found.");
    }

    Threadpool::AddTask(&Editor::CompileScript, path, &Editor::PostBuildScriptFromProject);
}

void Editor::PostBuildScriptFromProject() {

    Threadpool::AddTask([] {
        std::vector<std::string> cmd = {
            "/home/grsource/Documentos/gr-project/gr-engine-linux-project/bin/gr-engine-linux-project " +
                File::GetFile("")
        };
        
        Command::exec(cmd, [](std::string line) {
            LOG_WARNING(line);
        });
    });
}

void Editor::ReconfigScript() {
    LOG_INFO("[ BUILD ] Initialized reconfigation...");

    auto component = File::GetFile("/component", true);
    if (!File::Exists(component)) {
        return LOG_ERROR("[ BULD ] Falied to load internal data component.");
    }

    auto path = File::GetFile("/.build");
    if (File::Exists(path)) {
        File::Delete(path);
    }

    File::CreateDirectory(path);

    if (!File::Copy(component, path)) {
        return LOG_ERROR("[ BULD ]" + File::GetError());
    }

    LOG_INFO("[ BUILD ] building scipt...");

    Threadpool::AddTask(&Editor::BuildScript);
}

void Editor::BuildScript() {
    LOG_INFO("[ Build ] Initialize");

    auto path = File::GetFile("/.build");
    if (!File::Exists(path)) {
        return LOG_ERROR("[ Build ] Falied to build project, internal data not found.");
    }

    Threadpool::AddTask(&Editor::CompileScript, path, &Editor::LoadScript);
}

void Editor::CompileScript(std::string path, std::function<void()> func) {
    auto build = File::GetFile("/script/build.sh", true);
    if (!File::Exists(build)) {
        return LOG_ERROR("Build script do not found.");
    }

    auto component = File::GetFile("/.build/component");
    if (!File::Exists(component)) {
        return LOG_ERROR("Component library do not found. [1]");
    }

    std::vector<std::string> command{
        "export GR_ROOT_PROJECT_PATH='" + component + "'",
        "export GR_ROOT_ASSETS_PATH='" + File::GetAsset("") + "'",
        "chmod +x " + build,
        "sh " + build,
    };

    Command::exec(command, [](std::string line) {
        LOG_WARNING(line);
    });

    if (func)
        Event::Add(func);
}

void Editor::SaveScene(const std::string &name, Scene *scene) {
    Scene::Use(scene);

    scene->save(File::GetAsset("scene/") + name + ".scn");
}
