#include "editor.hpp"

#include "postProcessing.hpp"
#include "editorController.h"
#include "projectMenu.hpp"
#include "renderManager.h"
#include "screenRender.hpp"
#include "renderer.h"
#include "settings.h"
#include "engine.h"
#include "memory.h"
#include "skybox.h"

#include <gr-base/core/window.hpp>

#include <gr-base/core/filmbox/filmbox.hpp>

#include <gr-base/core/binary/model3DHandler.hpp>
#include <gr-base/core/binary/modelhandle.hpp>

#include <gr-base/core/file/file.hpp>

#include "core/cmd/cmd.hpp"

#include "core/file/initialization.hpp"
#include "core/file/config.hpp"

#include <gr-base/core/assetsManager.hpp>
#include <gr-base/core/threadpool.hpp>

#include <gr-base/core/console.hpp>
#include <gr-base/core/scene.hpp>
#include <gr-base/core/event.hpp>

#include "core/statistics.hpp"
#include "core/screen.hpp"
#include "core/timer.hpp"

#include <gr-base/script/boxCollider.hpp>
#include <gr-base/script/meshFilter.hpp>
#include <gr-base/script/meshRender.hpp>
#include <gr-base/script/transform.hpp>
#include <gr-base/script/rigidbody.hpp>
#include <gr-base/script/animator.hpp>
#include <gr-base/script/camera.hpp>
#include <gr-base/script/light.hpp>

#include "guizmo/ImGuizmo.h"

#include "ui/ImGuiRender.h"
#include "ui/project.hpp"
#include "ui/view.hpp"

#include "scriptManager/scriptManager.hpp"

#include <gr-base/core/geometry/batching.hpp>

#include <gr-base/ecs/gameObject.hpp>
#include <gr-base/ecs/manager.hpp>

#include <linux/input-event-codes.h>
#include <filesystem>
#include <sstream>
#include <iomanip>

#include <gr-base/ecs/componentRegistry.hpp>

#include <gr-base/core/display.hpp>

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

Editor::Editor() : m_viewport(ImGui::GetMainViewport()), mImGuiIO(&ImGui::GetIO()), m_currentPath(File::GetAsset("")), m_cameraObject(nullptr), m_mainCamera(nullptr), s_objectSelected(nullptr), m_currentPage(0), m_materialToRename(nullptr), tempCam(nullptr), m_plugin(nullptr) {
    s_texturePlayGame = nullptr;

    s_objectSelected = nullptr;

    m_currentPage = 0;

    popupFlags = 0;

    bFileDoubleClick = false;

    bCutAllFile = false;

    bCopyAllFile = false;

    bViewPortFocused = false;

    bOpenPopupInfoDir = false;

    bScriptReloaded = false;

    bOpenPopupOptionGameObject = false;

    FPSColor = {1.0f, 1.0f, 1.0f, 1.0f};

    m_lastViewportSize = {0.0f, 0.0f};

    m_guizmoOperation = ImGuizmo::TRANSLATE;

    m_guizmoManipulate = ImGuizmo::WORLD;

    s_maxProgress = 0;

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
        
        Settings::ViewportSize.x = m_config->hasKey("w") ? m_config->getValue<int>("w") : 0;
        Settings::ViewportSize.y = m_config->hasKey("h") ? m_config->getValue<int>("h") : 0;

        m_config->setCategory(lasCaterogy);
    }

    bBuildDock = File::Exists("imgui.ini");

    LoadReources();

    try {
        ScriptManager::Initialize(File::GetFile("/.build/component/lib/libgr-component.so"));

        Display::SetDisplayRect(Rect{0.0f, 0.0f, (float)Settings::ViewportSize.x, (float)Settings::ViewportSize.y});
    } catch (const std::exception &e) {
        Console::LogError("Fatal error in script: " + std::string(e.what()));
    }

    auto lastCategory = m_config->getCategory();

    m_config->setCategory("scene");
    if (m_config->hasKey("main") && m_config->getValue<std::string>("main") != "default" && !Scene::HasScene()) {
        Scene::LoadScene(File::GetAsset("/scene/" + m_config->getValue<std::string>("main") + ".scn"), static_cast<std::function<void(Scene *)>>(std::bind(&Editor::ScenePostEcecute, this, std::placeholders::_1)));
    }

    m_config->setCategory(lastCategory);

    Scene::Initialize();

    RenderManager::Initialize();

    ScreenRender::Initialize();

    Skybox::Initialize();

    auto project = Project::GetCurrent();
    if (project) {
        auto projectname = project->getName();

        char name[256];
        sprintf(name, "GR - %s", projectname.c_str());

        Window::SetWindowTitle(name);
    }
}

Editor::~Editor() {
    m_config->setCategory("scene");
    m_config->setValue<std::string>("main", Scene::HasScene() ?  Scene::GetCurrentScene()->getName() : "default");

    m_config->setCategory("viewport");
    m_config->setValue("w", Settings::ViewportSize.x);
    m_config->setValue("h", Settings::ViewportSize.y);

    if (s_objectSelected) {
        m_config->setCategory("editor");
        m_config->setValue("GoSelecetd", s_objectSelected->getName());
    }

    if (m_config) {
        m_config->save();
    }

    Scene::Shutdown();

    RenderManager::Shutdown();

    Skybox::Shutdown();

    ScreenRender::Shutdown();

    GameObject::Clear();

    ComponentRegistry::getInstance().clear();

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
    ModelHandle::ImportFBX(path);
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

    Display::SetDisplayRect(Rect{0.0f, 0.0f, (float)Settings::ViewportSize.x, (float)Settings::ViewportSize.y});

    RenderManager::Initialize();

    ScreenRender::Initialize();

    Scene::CheckRequestReinstanceScript();

    Settings::bGameRunning = true;

    Scene::InitializeAllScript();

    Console::log(Log::DEFAULT, "Game playing.");
}

void Editor::UnloadScript() {
    RenderManager::Shutdown();

    ScreenRender::Shutdown();

    ComponentRegistry::getInstance().clear();

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

    switch (popupFlags) {
    case EditorPopupFlags_LoadPlugin:
        ImGui::OpenPopup("##popup_load_plugin");
        popupFlags &= ~EditorPopupFlags_LoadPlugin;
        break;
    case EditorPopupFlags_AddComponent:
        ImGui::OpenPopup("##popup_add_component");
        popupFlags &= ~EditorPopupFlags_AddComponent;
        break;
    case EditorPopupFlags_PluginEditor:
        ImGui::OpenPopup("##popup_plugin_editor");
        popupFlags &= ~EditorPopupFlags_PluginEditor;
        break;
    case EditorPopupFlags_CreateMaterial:
        ImGui::OpenPopup("##popup_create_material");
        popupFlags &= ~EditorPopupFlags_CreateMaterial;
        break;
    case EditorPopupFlags_EngineStatus:
        PopupMenuProperty();
        break;
    case EditorPopupFlags_FileManager | EditorPopupFlags_DirManager:
        ImGui::OpenPopup("##popup_file_and_dir_manager");
        popupFlags &= ~(EditorPopupFlags_FileManager | EditorPopupFlags_DirManager);
        break;
    case EditorPopupFlags_FileManager:
        ImGui::OpenPopup("##popup_file_manager");
        popupFlags &= ~EditorPopupFlags_FileManager;
        break;
    case EditorPopupFlags_DirManager:
        ImGui::OpenPopup("##popup_dir_manager");
        popupFlags &= ~EditorPopupFlags_DirManager;
        break;
    case EditorPopupFlags_RenameFileOrDir:
        ImGui::OpenPopup("##popup_rename_file_or_dir");
        popupFlags &= ~EditorPopupFlags_RenameFileOrDir;
        break;
    case EditorPopupFlags_CreateNewDir:
        ImGui::OpenPopup("##popup_create_new_dir");
        popupFlags &= ~EditorPopupFlags_CreateNewDir;
        break;
    case EditorPopupFlags_CreateScript:
        ImGui::OpenPopup("##popup_create_script");
        popupFlags &= ~EditorPopupFlags_CreateScript;
        break;
    default:
        break;
    }
    
    PopupComponentMenu();

    PopupMenuSettings();

    showShaderListPopup();

    showMeshListPopup();

    PopupAddComponent();

    SkyboxEditor();

    PluginManager();

    PluginLoad();

    PluginEditor();

    RenameMaterial();

    CreateMaterial();

    PopupFileAndDirmanager();

    PopupFileManager();

    PopupDirManager();

    PopupRenameFileOrDir();

    PopupCreateDirectory();

    PopupCreateScript();
}

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

    EditorPopupFlags_ flags = 0;

    bool bPopupMenuShader = false, bMeshList = false;

    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("Project")) {
            if (ImGui::MenuItem("Reconfigure Script")) {
                ReconfigScript();
            }
            
            if (ImGui::MenuItem("Reconfigure Component")) {
                // ProjectMenu::CloseProject();
            }
            
            if (ImGui::MenuItem("Settings")) {}

            if (ImGui::MenuItem("Close")) {
                // ProjectMenu::CloseProject();
            }
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("View")) {
            bool isSelected = popupFlags & EditorPopupFlags_EngineStatus;
            if (ImGui::MenuItem("Engine status", nullptr, &isSelected)) {
                if (isSelected) {
                    popupFlags |= EditorPopupFlags_EngineStatus;
                } else {
                    popupFlags &= ~EditorPopupFlags_EngineStatus;
                }
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
                flags |= EditorPopupFlags_Settings;
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Tools")) {
            if (ImGui::MenuItem("Skybox Editor")) {
                flags |= EditorPopupFlags_SkyboxEditor;
            }

            if (ImGui::MenuItem("Plugin")) {
                flags |= EditorPopupFlags_Plugin;
            }

            // /*
            if (ImGui::MenuItem("Shader List")) {
                bPopupMenuShader = true;
            }

            if (ImGui::MenuItem("Mesh List")) {
                bMeshList = true;
            }
            // */
            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }

    ImGuiID dockspace_id = ImGui::GetID("DockSpace");
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
    ImGui::End();

    if (flags & EditorPopupFlags_Settings) {
        ImGui::OpenPopup("##popup_menu_settings");
    }

    if (flags & EditorPopupFlags_SkyboxEditor) {
        ImGui::OpenPopup("##popup_skybox_editor");
    }

    if (flags & EditorPopupFlags_Plugin) {
        ImGui::OpenPopup("##popup_plugin_manager");
    }

    // /*
    if (bPopupMenuShader) {
        ImGui::OpenPopup("##popup_menu_shader_list");
    }

    if (bMeshList) {
        ImGui::OpenPopup("##popup_menu_mesh_list");
    }
    // */

    if (bBuildDock) {
        return;
    }

    BuildDock(dockspace_id, dockspace_flags);

    bBuildDock = true;
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
    if (!isOpen) { return; }

    ImGui::EndPopup();
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
        Event::Add(&Scene::Destoy, s_objectSelected);
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
                Event::Add(&Scene::Destoy, s_objectSelected);
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
    
    auto bCreateScene = false;

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
            bCreateScene = true;
        }
        ImGui::EndTabBar();
    }

    if (bCreateScene) {
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
    UpdateResizeSceneFramebuffer();

    auto regionAvail = ImGui::GetContentRegionAvail();
    auto windowPos = ImGui::GetWindowPos();
    auto cursorPos = ImGui::GetCursorPos();

    auto texture = Engine::GetImGuiPostprocess()->getTextureColor();
    if (texture) {
        ImGui::Image(reinterpret_cast<ImTextureID>(texture->getID()), regionAvail, {0.0f, 1.0f}, {1.0f, 0.0f});

        if (ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
            auto mousePos = ImGui::GetMousePos();
            mousePos -= windowPos + cursorPos;

            mousePos.y = regionAvail.y - mousePos.y;

            int mouseX = static_cast<int>(mousePos.x);
            int mouseY = static_cast<int>(mousePos.y);

            if (mouseX >= 0 && mouseY >= 0 && mouseX <= static_cast<int>(regionAvail.x) && mouseY <= static_cast<int>(regionAvail.y)) {
                int entityID = -1;
                Engine::GetMainPostprocess()->readPixel(mouseX, mouseY, &entityID);

                if (entityID != -1) {
                    s_objectSelected = Manager::GetEntityFromID(static_cast<EntityID>(entityID))->getGameObject();
                }
            }
        }
        DropGameObjectInScene();
    }

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
        
        ImGui::SetCursorPos({regionAvail.x / 2 - cursorPos.x, cursorPos.y + 0.5f});

        if (ImGui::ImageButton(reinterpret_cast<ImTextureID>(s_texturePlayGame->getID()), ImVec2(16, 16))) {
            if (!Settings::bGameRunning) {
                PlayGame();
            } else {
                StopGame();
            }
        }
    }

    ImGuizmo::SetDrawlist();
    ImGuizmo::SetRect(windowPos.x + cursorPos.x, windowPos.y + cursorPos.y, regionAvail.x, regionAvail.y);

    if (m_mainCamera && s_objectSelected) {
        RenderGuizmo();
    }

    // Fps Draw
    {
        const char *fps_text = "FPS: 000.00f";

        auto textSize = ImGui::CalcTextSize(fps_text);

        ImGui::SetCursorPos(ImVec2(regionAvail.x - textSize.x, cursorPos.y + textSize.y));

        ImGui::TextColored(FPSColor, "FPS: %.2f", Statistics::m_threadFrame.fps);
    }

    ImGui::SetCursorPos({8.0f, cursorPos.y});

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

    if (ImGui::IsWindowFocused() && mImGuiIO->KeyCtrl && ImGui::IsKeyReleased(ImGuiKey_S)) {
        Event::Add(&Editor::SaveScene, this, Scene::GetCurrentScene()->getName(), Scene::GetCurrentScene());
    }
}

void Editor::UpdateResizeSceneFramebuffer() {
    ImVec2 regionAvail = ImGui::GetContentRegionAvail();
    if (m_mainCamera && (m_lastViewportSize.x != regionAvail.x || m_lastViewportSize.y != regionAvail.y && regionAvail.x > 5.0f && regionAvail.y > 5.0f || bScriptReloaded)) {
        auto &viewport = Settings::ViewportSize = {static_cast<int>(regionAvail.x), static_cast<int>(regionAvail.y)};
        if (viewport.x > 0 && viewport.y > 0) {
            Display::SetDisplayRect(Rect{0.0f, 0.0f, (float)Settings::ViewportSize.x, (float)Settings::ViewportSize.y});

            m_mainCamera->setAspectRatio(static_cast<float>(viewport.x) / static_cast<float>(viewport.y));

            Engine::GetMainPostprocess()->resize(viewport);
            
            m_lastViewportSize = regionAvail;
            bScriptReloaded = false;
        }
    }
}

void Editor::PlayGame() {
    Threadpool::AddTask(&Editor::BuildScript, this);
}

void Editor::StopGame() {
    Settings::bGameRunning = false;

    Scene::DestroyScript();

    Scene::CleanTemp();

    Scene::InitializeEngineScript();

    Editor::setActiveCamera(true);

    Console::log(Log::DEFAULT, "Game stoped.");
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
            ScenePostEcecute(Scene::Create(name));
        }
        
        ImGui::CloseCurrentPopup();
    }

    ImGui::EndPopup();
}

void Editor::RenderFilesUI() {
    ImGui::SetNextItemOpen(true, ImGuiCond_FirstUseEver);

    RenderTreeNodeFile(m_currentPath);
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

    if (mDirSelectedMap.find(path) == mDirSelectedMap.end()) {
        flags &= ~ImGuiTreeNodeFlags_Selected;
    }

    auto name = path.filename();
    
    auto isOpen = ImGui::TreeNodeEx(name.c_str(), flags);

    if (ImGui::IsItemClicked() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
        OpenFileDoubleClick(path);
    }

    auto isKeyDownShift = ImGui::IsKeyDown(ImGuiKey_LeftShift);
    if (isKeyDownShift) {
        if (ImGui::IsItemClicked(ImGuiMouseButton_Right) && !(flags & ImGuiTreeNodeFlags_Selected)) {
            mDirSelectedMap.emplace(path, true);
        } else if (ImGui::IsItemClicked(ImGuiMouseButton_Right) && (flags & ImGuiTreeNodeFlags_Selected)) {
            popupFlags |= EditorPopupFlags_FileManager | EditorPopupFlags_DirManager;
        }
    } else {
        mDirSelectedMap.clear();
    }

    if (!isKeyDownShift && mDirSelectedMap.empty() && ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
        mDirSelectedList.clear();

        mDirSelectedList.push_back(path);

        popupFlags |= bDirectory ? EditorPopupFlags_DirManager : EditorPopupFlags_FileManager;
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

void Editor::PopupFileAndDirmanager() {
    auto isOpen = ImGui::BeginPopup("##popup_file_and_dir_manager");
    if (!isOpen) {
        return;
    }

    for (auto &&[path, isSeleceted] : mDirSelectedMap) {
        ImGui::Text("Dir: %s", path.filename().string().c_str());
    }

    //bool bCreateDiretory = false, bCreateScript = false, bRenameFile = false, bCreateMaterial = false;;

    std::array<const char *, 1> item_list{"Delete"};

    int i = 0;
    if (ImGui::ListBox("##DirManager", &i, item_list.data(), item_list.size())) {
        switch (i) {
        case 0:
            for (auto &&[path, isSeleceted] : mDirSelectedMap) {
                File::Delete(path);
            }
            break;
        /*
        case 1:
            bRenameFile = true;
            break;
        case 2:
            bCreateDiretory = true;
            break;
        case 3:
            bCreateScript = true;
            break;
        case 4:
            popupFlags |= EditorPopupFlags_CreateMaterialInDir;
            break;
            */
        default:
            break;
        }
        mDirSelectedMap.clear();
        ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();

    /*
    if (bRenameFile) {
        ImGui::OpenPopup("##popup_rename_file");
    }

    if (bCreateDiretory) {
        ImGui::OpenPopup("##popup_create_new_dir");
    }

    if (bCreateScript) {
        ImGui::OpenPopup("##popup_create_new_script");
    }
    */
}

void Editor::PopupFileManager() {
    bool isOpenPopup = ImGui::BeginPopup("##popup_file_manager");
    if (!isOpenPopup) { return; }

    auto path = mDirSelectedList.back();

    ImGui::Text("%s", path.filename().c_str());

    const char *item_list[4] {"Delete", "Rename", "Move", "Copy"};

    int i = 0;
    if (ImGui::ListBox("##FileManager", &i, item_list, 4)) {
        switch (i) {
        case 0:
            File::Delete(path);
            mDirSelectedList.clear();
            break;
        case 1:
            popupFlags |= EditorPopupFlags_RenameFileOrDir;
            break;
        default:
            break;
        }
        ImGui::CloseCurrentPopup();
    }

    ImGui::EndPopup();
}

void Editor::PopupDirManager() {
    bool isOpenPopup = ImGui::BeginPopup("##popup_dir_manager");
    if (!isOpenPopup) { return; }

    auto path = mDirSelectedList.back();

    ImGui::Text("%s", path.filename().c_str());

    const char *item_list[6] {"Delete", "Rename", "Create Dir", "Create Script", "Move", "Copy"};

    int i = 0;
    if (ImGui::ListBox("##FileManager", &i, item_list, 4)) {
        switch (i) {
        case 0:
            File::Delete(path);
            mDirSelectedList.clear();
            break;
        case 1:
            popupFlags |= EditorPopupFlags_RenameFileOrDir;
            break;
        case 2:
            popupFlags |= EditorPopupFlags_CreateNewDir;
            break;
        case 3:
            popupFlags |= EditorPopupFlags_CreateScript;
            break;
        default:
            break;
        }
        ImGui::CloseCurrentPopup();
    }

    ImGui::EndPopup();
}

void Editor::PopupRenameFileOrDir() {
    auto isOpen = ImGui::BeginPopup("##popup_rename_file_or_dir");
    if (!isOpen) {
        return;
    }
    auto path = mDirSelectedList.back();

    ImGui::Text("Rename:");
    if (!ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked(0))
        ImGui::SetKeyboardFocusHere(0);


    auto name = path.filename().string();
    if (name.size() < 256) {
        name.resize(256, '\0');
    }

    if (ImGui::InputText("##popup_rename_file", name.data(), name.size(), ImGuiInputTextFlags_EnterReturnsTrue)) {
        name.erase(std::remove(name.begin(), name.end(), '\0'), name.end());

        auto temp_path = path;
        File::rename(path, temp_path.replace_filename(name));

        mDirSelectedList.clear();
        
        ImGui::CloseCurrentPopup();
    }

    ImGui::EndPopup();
}

void Editor::PopupCreateDirectory() {
    auto isOpen = ImGui::BeginPopup("##popup_create_new_dir");
    if (!isOpen) {
        return;
    }

    auto path = mDirSelectedList.back();
    
    ImGui::Text("Input directory name:");
    if (!ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked(0))
        ImGui::SetKeyboardFocusHere(0);

    char name[256] = {0};
    if (ImGui::InputText("##InputDirectoryName", name, 256, ImGuiInputTextFlags_EnterReturnsTrue)) {
        File::CreateDirectory(path.string() + "/" + std::string(name));

        mDirSelectedList.clear();
        ImGui::CloseCurrentPopup();
    }

    ImGui::EndPopup();
}

void Editor::PopupCreateScript() {
    auto isOpen = ImGui::BeginPopup("##popup_create_script");
    if (!isOpen) {
        return;
    }

    auto path = mDirSelectedList.back();

    ImGui::Text("Input your script name");

    char name[256] = {0};
    if (ImGui::InputText("##input_text_popup_create_script_name", name, 256, ImGuiInputTextFlags_EnterReturnsTrue)) {
        CreateScriptTemplate(path.string(), name);

        mDirSelectedList.clear();

        ImGui::CloseCurrentPopup();
    }
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
        color = {0.1f, 0.8f, 0.1f, 1.0f};
        break;
    case Log::LogType::WARNING:
        color = {0.8f, 0.8f, 0.1f, 1.0f};
        break;
    case Log::LogType::ERROR:
        color = {0.8f, 0.1f, 0.1f, 1.0f};
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

#include <gr-base/core/status.hpp>

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
    ImGui::Text("Luzes: %ld",     Light::GetListLight().size());
    ImGui::Text("BatchRender: %ld/%ld", Batching::GetBatchCount(), Status::BatchRenderCount);
}

void Editor::RenderGameObjectPropertiUI() {
    if (!s_objectSelected) { return; }

    auto entity = s_objectSelected->getEntity();

    auto objName = s_objectSelected->getName();
    ImGui::PushID(objName.c_str());

    ImGui::Image(reinterpret_cast<ImTextureID>(iconsUI[IconType::ICON_GAMEOBJECT]->getID()), ImVec2(16, 16));

    ImGui::SameLine();

    ImGui::Text("[ %s - %s ]", objName.c_str(), s_objectSelected->getTag().c_str());

    ImGui::SameLine();

    auto isActive = s_objectSelected->getActive();
    if (ImGui::Checkbox("##check_box_is_active", &isActive)) {
        s_objectSelected->setActive(isActive);
    }

    for (auto [name, component] : entity->getComponentMap()) {
        ImGui::PushID(name.c_str());

        if (name == "Transform") {
            RenderProperty<Transform>(&Editor::OnPropertyTransform, this);
            ImGui::PopID();
            continue;
        }

        if (name == "Camera") {
            RenderProperty<Camera>(&Editor::OnPropertyCamera, this);
            ImGui::PopID();
            continue;
        }
        
        if (name == "Light") {
            if (RenderProperty<Light>(&Editor::OnPropertyLight, this)) {
                entity->destroy(name);
            }
            ImGui::PopID();
            continue;
        }

        if (name == "MeshRender") {
            if (RenderProperty<MeshRender>(&Editor::OnPropertyMeshRender, this)) {
                entity->destroy(name);
            }
            ImGui::PopID();
            continue;
        }

        if (name == "MeshFilter") {
            if (RenderProperty<MeshFilter>(&Editor::OnPropertyMeshFilter, this)) {
                entity->destroy(name);
            }
            ImGui::PopID();
            continue;
        }

        if (name == "BoxCollider") {
            if (RenderProperty<BoxCollider>(&Editor::OnPropertyBoxCollider, this)) {
                entity->destroy(name);
            }
            ImGui::PopID();
            continue;
        }

        if (name == "Rigidbody") {
            if (RenderProperty<Rigidbody>(&Editor::OnPropertyRigidbody, this)) {
                entity->destroy(name);
            }
            ImGui::PopID();
            continue;
        }

        if (name == "Animator") {
            if (RenderProperty<Animator>(&Editor::OnPropertyAnimator, this)) {
                entity->destroy(name);
            }
            ImGui::PopID();
            continue;
        }
        
        if (!RenderComponents(name, &(*component))) {
            entity->destroy(name);
            ImGui::PopID();
            continue;
        }
        ImGui::PopID();
    }

    auto avail = ImGui::GetContentRegionAvail();

    if (ImGui::Button("AddComponent", {avail.x, 0.0f})) {
        popupFlags |= EditorPopupFlags_AddComponent;
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
    
    auto fieldList = it->second.pointer->getFields();

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
    case FieldType_GameObject: {
        renderDropGameObject(Field::GetFieldData<GameObject *>(component, field));
        // auto gameObject = Field::GetFieldData<GameObject *>(component, field);
        // *gameObject = s_objectSelected;

        // if (*gameObject)
        //     ImGui::Text("%s", (*gameObject)->getName().c_str());
        // dropGameObject();
        break;
    }
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

void Editor::renderDropGameObject(GameObject **ref) {
    if (*ref) {
        ImGui::Text("%s", (*ref)->getName().c_str());
    }

    if (!ImGui::BeginDragDropTarget()) {
        return;
    }
    auto data = ImGui::AcceptDragDropPayload(IMGUI_PAYLOAD_FILE, ImGuiDragDropFlags_None);
    if (data) {
        std::string path((char*)data->Data, data->DataSize);
        if (path.empty() || path.find(".gmdx") == std::string::npos) {
            return ImGui::EndDragDropTarget();
        }
        return AssetsManager::LoadGameObject(path, &Editor::OutputGameObject, ref);
    }

    ImGui::EndDragDropTarget();
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
    auto bSelect = ImGui::InputText("##popup_add_componnet_inputtext_filter", componentName, sizeof(componentName), ImGuiInputTextFlags_EnterReturnsTrue);

    m_componentName.erase(std::remove_if(m_componentName.begin(), m_componentName.end(), [] (auto &name) {
        return std::strstr(name.c_str(), componentName) == nullptr;
    }), m_componentName.end());

    auto regionAvail = ImGui::GetContentRegionAvail();

    for (auto it = m_componentName.begin(); it != m_componentName.end(); ++it) {
        bool isFirst = (it == m_componentName.begin());
        if (ImGui::Selectable(it->c_str(), isFirst) || bSelect) {
            auto component = s_objectSelected->getEntity()->addComponent((*it));

            if (component && (*it) == "Light") {
                component->Init();

                std::cout << "X" << std::endl;
            }

            std::memset(componentName, '\0', sizeof(componentName));

            ImGui::CloseCurrentPopup();
            
            break;
        }
    }
    
    ImGui::EndPopup();
}

void Editor::PopupMenuProperty() {
    ImGui::Begin("Status");

    RenderStatisticsMenu();

    ImGui::End();
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

bool Editor::OnPropertyCamera(Camera *camera) {
    bool bactive = true;
    auto isOpen = ImGui::CollapsingHeader("Camera", &bactive);
    if (!bactive) {
        return true;
    }

    if (!isOpen) {
        return false;
    }

    bool isChanged = false;

    ImGui::Text("Rect");
    auto rect = camera->getRect();

    ImGui::Text("x");
    isChanged |= ImGui::DragFloat("##camera_rect_x", &rect.x);

    ImGui::Text("y");
    isChanged |= ImGui::DragFloat("##camera_rect_y", &rect.y);

    ImGui::Text("w");
    isChanged |= ImGui::DragFloat("##camera_rect_w", &rect.w);

    ImGui::Text("h");
    isChanged |= ImGui::DragFloat("##camera_rect_h", &rect.h);

    if (isChanged) {
        camera->setRect(rect);
    }

    auto texture = camera->getColorTexture();
    if (!texture) { return false; }

    if (ImGui::TreeNode("Render")) {
        ImGui::Image(reinterpret_cast<ImTextureID>(texture->getID()), {64.0f, 64.0f}, {0.0f, 1.0f}, {1.0f, 0.0f});

        ImGui::TreePop();
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

    ImGui::Text("Name: %s", meshFilter->getMesh()->getName().c_str());

    return false;
}

void Editor::RenderUVConfig(MeshFilter *meshFilter) {
    auto mesh = meshFilter->getMesh();

    auto isOpen = ImGui::TreeNode("UV");
    if (!isOpen || !mesh) {
        return;
    }
    const char *name[] {"X", "Y", "Z"};

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
   
    auto materialList = meshRender->getMaterials();

    auto avail = ImGui::GetContentRegionAvail();
    
    if (ImGui::Button("Create", {avail.x, 0.0f})) {
        popupFlags |= EditorPopupFlags_CreateMaterial;
    }

    ImGui::Indent();
    for (auto material : materialList) {
        if (RenderMaterial(material)) {
            meshRender->removeMaterial(material);
        }
    }
    ImGui::Unindent();

    if (m_materialToRename) {
        ImGui::OpenPopup("##popup_rename_material");
    }

    return false;
}

bool Editor::RenderMaterial(Material *material) {
    auto bVisible = true;

    auto name = AssetsManager::GetRefMaterialToName(material);

    auto isOpen = ImGui::CollapsingHeader(name.c_str(), &bVisible);
    if (!bVisible) {
        return true;
    }
    
    if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
        m_materialToRename = material;
    }

    if (!isOpen) {
        return false;
    }

    auto albedo = material->getAlbedo();
    InsertTexture(material);

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
    AssetsManager::LoadTexture(path, &Material::setTexture, material, 0);
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

    auto freese = rigidbody->getFreezerRotation();
    ImGui::Text("Freeze Rotation");
    if (ImGui::DragFloatEx("##rigidbody_freeze", text, reinterpret_cast<float *>(&freese), 1.0f, 0.0f, 0.0f, "%.3f")) {
        rigidbody->setFreezerRotation(freese);
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
    if (!Camera::Current()) {
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
    
    Scene::Init(gameObject);

    s_objectSelected = gameObject;
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

// TODO: Precisa adicionar um suport para renoemar o marterial
void Editor::RenameMaterial() {
    /*
    auto isOpen = ImGui::BeginPopupModal("##popup_rename_material");
    if (!isOpen || !m_materialToRename) {
        return;
    }

    ImGui::Text("Rename");
    if (!ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked(0))
        ImGui::SetKeyboardFocusHere(0);

    auto name = AssetsManager::GetRefMaterialToName(m_materialToRename);
    if (name.size() < 256) {
        name.resize(256, '\0');
    }

    if (ImGui::InputText("##popup_rename_file", name.data(), name.size(), ImGuiInputTextFlags_EnterReturnsTrue)) {
        name.erase(std::remove(name.begin(), name.end(), '\0'), name.end());

        // m_materialToRename->setName(name);

        m_materialToRename = nullptr;

        ImGui::CloseCurrentPopup();
    }

    ImGui::EndPopup();
    */
}

void Editor::CreateMaterial() {
    auto isOpen = ImGui::BeginPopupModal("##popup_create_material");
    if (!isOpen || !s_objectSelected) {
        return;
    }

    ImGui::Text("Name");
    if (!ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked(0))
        ImGui::SetKeyboardFocusHere(0);

    static char name[256] = {0};
    if (ImGui::InputText("##popup_create_material_inputtext", name, 256, ImGuiInputTextFlags_EnterReturnsTrue)) {
        auto meshRender = s_objectSelected->getComponent<MeshRender>();
        
        meshRender->addMaterial(name);

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

        m_mainCamera->setActive(true);
        return;
    }
    m_mainCamera->setActive(false);

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

    auto regionAvail = ImGui::GetContentRegionAvail();
    const char *reload_text = "Reload";
    auto textSize = ImGui::CalcTextSize(reload_text);

    auto button_pos_x = regionAvail.x - textSize.x;

    for (auto &&[name, shader] : shader_list) {
        ImGui::Text("%s", name.c_str());

        ImGui::SameLine();

        ImGui::SetCursorPosX(button_pos_x);
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

        std::vector<int> indices = {
            0, 1, 5,  5, 1, 6,
            1, 2, 6,  6, 2, 7,
            2, 3, 7,  7, 3, 8,
            3, 4, 8,  8, 4, 9,
            10, 11, 0,  0, 11, 1,
            5, 6, 12, 12, 6, 13
        };

        std::vector<Vertex3D> vertices = {
            Vertex3D(Vector3{-1, -1, -1}, Vector3{0, 0, 0}, Vector2{0, 0}, 0),
            Vertex3D(Vector3{ 1, -1, -1}, Vector3{0, 0, 0}, Vector2{1, 0}, 0),

            Vertex3D(Vector3{ 1,  1, -1}, Vector3{0, 0, 0}, Vector2{2, 0}, 0),
            Vertex3D(Vector3{-1,  1, -1}, Vector3{0, 0, 0}, Vector2{3, 0}, 0),

            Vertex3D(Vector3{-1, -1, -1}, Vector3{0, 0, 0}, Vector2{4, 0}, 0),
            Vertex3D(Vector3{-1, -1,  1}, Vector3{0, 0, 0}, Vector2{0, 1}, 0),

            Vertex3D(Vector3{ 1, -1,  1}, Vector3{0, 0, 0}, Vector2{1, 1}, 0),
            Vertex3D(Vector3{ 1,  1,  1}, Vector3{0, 0, 0}, Vector2{2, 1}, 0),

            Vertex3D(Vector3{-1,  1,  1}, Vector3{0, 0, 0}, Vector2{3, 1}, 0),
            Vertex3D(Vector3{-1, -1,  1}, Vector3{0, 0, 0}, Vector2{4, 1}, 0),

            Vertex3D(Vector3{-1,  1, -1}, Vector3{0, 0, 0}, Vector2{0, -1}, 0),
            Vertex3D(Vector3{ 1,  1, -1}, Vector3{0, 0, 0}, Vector2{1, -1}, 0),

            Vertex3D(Vector3{-1,  1,  1}, Vector3{0, 0, 0}, Vector2{0,  2}, 0),
            Vertex3D(Vector3{ 1,  1,  1}, Vector3{0, 0, 0}, Vector2{1,  2}, 0)
        };

        for (std::size_t i=0;i<indices.size();i+=3) {
            auto A = vertices[indices[i] - 1].position;
            auto B = vertices[indices[i + 1] - 1].position;
            auto C = vertices[indices[i + 2] - 1].position;

            auto AB = B - A;
            auto AC = C - A;
            auto ABxAC = Math::cross(AB, AC);
         
            vertices[indices[i] - 1].normal = Math::normalize(vertices[indices[i] - 1].normal + ABxAC);
            vertices[indices[i + 1] - 1].normal = Math::normalize(vertices[indices[i + 1] - 1].normal + ABxAC);
            vertices[indices[i + 2] - 1].normal = Math::normalize(vertices[indices[i + 2] - 1].normal + ABxAC);
        }

        mesh->setIndices(indices);
        mesh->setVertices(vertices);
    }

    auto mf = go->addComponent<MeshFilter>();
    mf->Init();
    mf->setMesh(mesh);

    auto mr = go->addComponent<MeshRender>();
    mr->Init();

    mr->addMaterial("DefaultMaterial");
    // mr->build();

    return GameObject::Instantiate(go);
}

void Editor::SkyboxEditor() {
    auto isOpen = ImGui::BeginPopupModal("##popup_skybox_editor", nullptr, ImGuiWindowFlags_Modal);
    if (!isOpen) { return; }

    ImGui::Text("Teste");

    ImGui::EndPopup();
}

#include "plugin/pluginManager.hpp"

void Editor::PluginManager() {
    ImVec2 winSize(200, 300);
    
    ImGui::SetNextWindowPos(ImVec2{static_cast<float>(Screen::width / 2), static_cast<float>(Screen::height / 2)} - (winSize / 2.0f), ImGuiCond_Always);
    ImGui::SetNextWindowSize(winSize, ImGuiCond_Always);
    auto isOpen = ImGui::BeginPopup("##popup_plugin_manager");
    if (!isOpen) { return; }

    auto avail = ImGui::GetContentRegionAvail();

    for (auto plugin : PluginManager::getPluginList()) {
        auto info = plugin->getPluginInfo();

        char buff[512];
        sprintf(buff, "%s\n%s", info.name.c_str(), info.description.c_str());

        if (ImGui::Button(buff, ImVec2{avail.x, 0.0f})) {
            m_plugin = plugin;
            
            popupFlags |= EditorPopupFlags_PluginEditor;

            ImGui::CloseCurrentPopup();
        }

        ImGui::Separator();
    }

    auto calcTexsize = ImGui::CalcTextSize("Install");

    ImGui::SetCursorPosY(avail.y - calcTexsize.y);
    if (ImGui::Button("Install", ImVec2{avail.x, 0.0f})) {
        popupFlags |= EditorPopupFlags_LoadPlugin;

        ImGui::CloseCurrentPopup();
    }

    ImGui::EndPopup();
}

void Editor::PluginLoad() {
    ImVec2 winSize(200, 300);
    
    ImGui::SetNextWindowPos(ImVec2{static_cast<float>(Screen::width / 2), static_cast<float>(Screen::height / 2)} - (winSize / 2.0f), ImGuiCond_Always);
    ImGui::SetNextWindowSize(winSize, ImGuiCond_Always);
    auto isOpen = ImGui::BeginPopup("##popup_load_plugin");
    if (!isOpen) { return; }

    auto avail = ImGui::GetContentRegionAvail();

    static auto home_dir = File::getHomeDirectory();

    for (auto entry : std::filesystem::directory_iterator(home_dir)) {
        auto path = entry.path();
        if (path.filename().string()[0] == '.' || !std::filesystem::exists(path)) { continue; }

        if (ImGui::Button(path.filename().c_str(), ImVec2{avail.x, 0.0f})) {
            if (std::filesystem::is_directory(path)) {
                home_dir = path;
                continue;
            }
            
            if (path.extension() == ".gp") {
                popupFlags &= ~EditorPopupFlags_LoadPlugin;

                ImGui::CloseCurrentPopup();

                PluginManager::Load(path.string());
                
                return ImGui::EndPopup();
            }
        }
    }

    if (ImGui::IsKeyReleased(ImGuiKey_Escape)) {
        if (home_dir == File::getHomeDirectory()) {
            popupFlags &= ~EditorPopupFlags_LoadPlugin;

            ImGui::CloseCurrentPopup();

            return ImGui::EndPopup();
        }
        home_dir = std::filesystem::path(home_dir).parent_path();
    }

    ImGui::EndPopup();
}

void Editor::PluginEditor() {
    if (!m_plugin) { return; }

    ImVec2 winSize(200, 300);
    
    ImGui::SetNextWindowPos(ImVec2{static_cast<float>(Screen::width / 2), static_cast<float>(Screen::height / 2)} - (winSize / 2.0f), ImGuiCond_Always);
    ImGui::SetNextWindowSize(winSize, ImGuiCond_Always);

    auto isOpen = ImGui::BeginPopup("##popup_plugin_editor");
    if (!isOpen) { return; }

    auto avail = ImGui::GetContentRegionAvail();

    auto info = m_plugin->getPluginInfo();

    ImGui::SeparatorText(info.name.c_str());

    ImGui::Text("%s", info.description.c_str());

    ImGui::Spacing();

    auto textSize = ImGui::CalcTextSize("Remove");
    ImGui::SetCursorPosY(avail.y - textSize.y);

    if (ImGui::Button("Remove", ImVec2{avail.x, 0.0f})) {
        PluginManager::Remove(m_plugin);
        
        m_plugin = nullptr;
    }

    ImGui::EndPopup();
}

void Editor::ScenePostEcecute(Scene *scene) {
    m_cameraObject = scene->getCameraObject();

    m_mainCamera = scene->getCamera();
    // m_config->setCategory("editor");
    // if (!m_config->hasKey("GoSelecetd")) { return; }

    // s_objectSelected = GameObject::Find(m_config->getValue<std::string>("GoSelecetd"));
}

void Editor::OutputGameObject(GameObject **ref, GameObject *go) {
    *ref = go;
}
