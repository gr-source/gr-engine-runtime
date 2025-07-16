#include "editor.hpp"

#include <cstring>
#include <gr-base/script/baserender.hpp>
#include <gr-base/script/default.hpp>

#include "Global.h"
#include "core/time.hpp"
#include "editorController.h"
#include "gr-base/core/clock.hpp"
#include "gr-base/core/geometry/shader.hpp"
#include "gr-base/core/mainthreaddispatcher.hpp"
#include "gr-base/core/task/runtime/gameobjectsavetask.hpp"
#include "gr-base/core/task/runtime/scenesavetask.hpp"
#include "gr-base/core/thread/asynctask.hpp"
#include "gr-base/core/thread/taskmanager.hpp"
#include "physics/physicsScene.hpp"
#include "screenRender.hpp"
#include "settings.h"
#include "imgui.h"

#include <gr-base/core/window.hpp>

#include <gr-base/core/filmbox/filmbox.hpp>

#include <gr-base/core/binary/modelhandle.hpp>

#include <gr-base/core/file/file.hpp>

#include <gr-base/core/thread/threadpool.hpp>

#include <gr-base/core/assetsManager.hpp>
#include <gr-base/core/scenemanager.hpp>
#include <gr-base/core/console.hpp>
#include <gr-base/core/display.hpp>
#include <gr-base/core/scene.hpp>

#include "plugin/pluginManager.hpp"

#include "core/statistics.hpp"
#include "core/screen.hpp"

#include "core/cmd/cmd.hpp"

#include <gr-base/script/skinnedMeshRender.hpp>
#include <gr-base/script/meshCollider.hpp>
#include <gr-base/script/boxCollider.hpp>
#include <gr-base/script/meshFilter.hpp>
#include <gr-base/script/meshRender.hpp>
#include <gr-base/script/transform.hpp>
#include <gr-base/script/rigidbody.hpp>
#include <gr-base/script/animator.hpp>
#include <gr-base/script/camera.hpp>
#include <gr-base/script/light.hpp>

#include <gr-base/ecs/entitymanager.hpp>
#include <gr-base/ecs/gameObject.hpp>

#include "guizmo/ImGuizmo.h"

#include "ui/ImGuiRender.h"
#include "ui/project.hpp"
#include "ui/view.hpp"

#include "scriptManager/scriptManager.hpp"

#include <gr-base/core/geometry/batching.hpp>

#include <gr-base/ecs/componentRegistry.hpp>

#include <gr-ini/config.h>
#include <gr-math/matrix4x4.hpp>
#include <gr-render/gTexture.h>
#include <filesystem>

#include <pstl/glue_algorithm_defs.h>
#include <stdexcept>
#include <vector>

#define ITEMS_PER_PAGE 20

#define GET_N_ARRAY(c) sizeof(c) / sizeof(c[0])

enum class DragDropFlags : uint32_t {
    GMDX        = 1 << 1,
    SCN         = 1 << 2,
    SCRIPT      = 1 << 3,
    SHADER      = 1 << 4,
    GAMEOBJECT  = 1 << 5,
    ANIMATION   = 1 << 6,
    TEXTURE     = 1 << 7,
};

static std::map<DragDropFlags, const char *> m_extensionDragDrop =
{
    {DragDropFlags::GMDX,       ".gmdx"},
    {DragDropFlags::SCN,        ".scn"},
    {DragDropFlags::SCRIPT,     ".script"},
    {DragDropFlags::SHADER,     ".shader"},
    {DragDropFlags::GAMEOBJECT, ".gameobject"},
    {DragDropFlags::ANIMATION,  ".anm"},
    {DragDropFlags::TEXTURE,    ".png"},
};

static std::unordered_map<std::string, DragDropFlags> m_extensionNameToFlag =
{
    {".gmdx",       DragDropFlags::GMDX},
    {".scn",        DragDropFlags::SCN},
    {".cpp",        DragDropFlags::SCRIPT},
    {".hpp",        DragDropFlags::SCRIPT},
    {".glsl",       DragDropFlags::SHADER},
    {".gameobject", DragDropFlags::GAMEOBJECT},
    {".anm",        DragDropFlags::ANIMATION},
    {".png",        DragDropFlags::TEXTURE},
};

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

#define INSERT_SCRIPT(object) \
    {#object, [this](Entity *entity) \
        { \
            if (RenderProperty<object>(&Editor::OnProperty##object, this)) \
            { \
                entity->destroy(#object); \
            } \
        } \
    }

Editor::Editor() : m_viewport(ImGui::GetMainViewport()), mImGuiIO(&ImGui::GetIO()), m_currentPath(File::GetAsset("")), m_cameraObject(nullptr), m_mainCamera(nullptr), m_currentPage(0), m_materialToRename(nullptr), m_plugin(nullptr)
{
    m_scene = nullptr;

    s_texturePlayGame = nullptr;

    g_tree_file = nullptr;

    m_currentPage = 0;

    popupFlags = 0;

    m_dirSelected = m_currentPath;

    FPSColor = {1.0f, 1.0f, 1.0f, 1.0f};

    m_lastViewportSize = {0.0f, 0.0f};

    m_guizmoOperation = ImGuizmo::TRANSLATE;

    m_guizmoManipulate = ImGuizmo::WORLD;

    mInternalScript = {
        {"Transform", [this](Entity *entity)
            {
                RenderProperty<Transform>(&Editor::OnPropertyTransform, this);
            }
        },
        INSERT_SCRIPT(Camera),
        INSERT_SCRIPT(Light),
        INSERT_SCRIPT(MeshRender),
        INSERT_SCRIPT(SkinnedMeshRender),
        INSERT_SCRIPT(MeshFilter),
        // INSERT_SCRIPT(MeshCollider),
        INSERT_SCRIPT(BoxCollider),
        INSERT_SCRIPT(Rigidbody),
        INSERT_SCRIPT(Animator),
    };
    
    m_viewport = ImGui::GetMainViewport();

    m_settings = config_create();
    if (File::Exists(File::GetFile("/gsettings.ini")))
    {
        config_load(File::GetFile("/gsettings.ini").c_str(), m_settings);
            
        config_foreach(m_settings, static_cast<ConfigFunc>(&Editor::foreach_settings), this);
    } else
    {
        generateConfig(File::GetFile("/gsettings.ini"));
    }

    bBuildDock = File::Exists("imgui.ini");

    LoadReources();

    try
    {
        auto section_script = config_find_section(m_settings, "Script");
        if (section_script != nullptr)
        {
            ScriptManager::Load(File::GetFile(config_get_string(section_script, "dir")) + config_get_string(section_script, "libname"));
        }

        Display::SetDisplayRect(Rect{0.0f, 0.0f, (float)Settings::ViewportSize.x, (float)Settings::ViewportSize.y});
    } catch (const std::exception &e)
    {
        Console::LogError("Fatal error in script: %s", e.what());
    }

    root_tree_file.path = File::GetAsset("");
    root_tree_file.name = "Assets";
    root_tree_file.parent = nullptr;
    root_tree_file.bdirectory = true;
    root_tree_file.icon = nullptr;

    updateFileTree(root_tree_file);

    auto project = Project::GetCurrent();
    if (project)
    {
        auto projectname = project->getName();

        char name[256];
        sprintf(name, "GR - %s", projectname.c_str());

        Window::SetWindowTitle(name);
    }
}

Editor::~Editor()
{
    generateConfig(File::GetFile("/gsettings.ini"));

    config_save(File::GetFile("/gsettings.ini").c_str(), m_settings);

    config_release(m_settings);

    ComponentRegistry::Clear();

    Console::Shutdown();

    File::Delete(File::GetTempFile("gr-engine"));
}

void Editor::LoadScript()
{
    if (m_scene == nullptr)
    {
        return;
    }

    auto section_script = config_find_section(m_settings, "Script");

    ScriptManager::Unload();
    
    if (ScriptManager::CheckScriptLoaded(config_get_string(section_script, "libname")))
    {
        return Console::LogError("[ Script ] Fatal error data not unloaded.");
    }

    try
    {
        if (section_script != nullptr)
        {
            ScriptManager::Load(File::GetFile(config_get_string(section_script, "dir")) + config_get_string(section_script, "libname"));
        }

        ComponentRegistry::CheckScriptRemoved();
    } catch (const std::runtime_error &e)
    {
        Console::LogError(e.what());
    }

    Display::SetDisplayRect(Rect{0.0f, 0.0f, (float)Settings::ViewportSize.x, (float)Settings::ViewportSize.y});

    Scene::CheckRequestReinstanceScript();

    m_mainCamera->setActive(false);

    m_scene->play();

    Settings::bGameRunning = true;

    TaskManager::Add<PhysicsTask>();

    Console::log(Log::DEFAULT, "Game playing.");
}

void Editor::UnloadScript() {
    ScreenRender::Shutdown();
}

// ===================================== END =========================================================
void Editor::onRender()
{
    create_dock();

    create_window_viewport();

    create_window_hierarchy();

    create_window_browser();

    create_window_console();

    create_window_property();

    create_call_popup();

    create_popup();
}

void Editor::onFocus()
{
    // m_scene = Scene::GetCurrentScene();
}

void Editor::onBlur()
{
}

void Editor::drop(void *data)
{
    updateFileTree(root_tree_file);
}

void Editor::create_dock()
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("Project"))
        {
            if (ImGui::MenuItem("Rebuild Script"))
            {
                ReconfigScript();
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("View"))
        {
            if (ImGui::MenuItem("Status"))
            {
            }
        }

        ImGui::EndMainMenuBar();
    }

    PopupMenuProperty();
    
    if (bBuildDock)
    {
        return;
    }

    // BuildDock(dockID, 0);

    bBuildDock = true;
}

void Editor::BuildDock(ImGuiID dockspace_id, ImGuiDockNodeFlags dockspace_flags)
{
    ImGui::DockBuilderRemoveNode(dockspace_id);
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

void Editor::PopupComponentMenu()
{
    auto isOpen = ImGui::BeginPopup("##game_object_component_menu");
    if (!isOpen)
    {
        return;
    }

    ImGui::EndPopup();
}

void Editor::create_window_hierarchy()
{
    if (ImGui::Begin("hierarchy"))
    {
        create_render_hierarchy();
    }
    ImGui::End();
}

void Editor::create_render_hierarchy()
{
    std::string name = m_scene != nullptr ? SceneManager::GetSceneName(m_scene) : "Scene name is empty!";

    ImGui::SetNextItemOpen(true, ImGuiCond_Appearing);
    if (m_scene != nullptr && ImGui::TreeNodeEx(name.c_str(), ImGuiTreeNodeFlags_SpanFullWidth))
    {
        dropGameObjectInHierarchy(m_scene);
        
        RenderSceneObject(m_scene);

        ImGui::TreePop();
    }

    if (ImGui::IsKeyReleased(ImGuiKey_Delete) && m_scene->has_object_selected())
    {
        for (auto go : m_scene->get_object_selected())
            m_scene->destroy(go);

        m_scene->deselect_all();
    }
    
    if (ImGui::IsKeyReleased(ImGuiKey_F2) && ImGui::IsWindowHovered() && m_scene->has_object_selected())
    {
        popupFlags |= EditorPopupFlags_RenameGameObject;
    }

    if (ImGui::IsKeyReleased(ImGuiKey_Escape) && ImGui::IsWindowHovered())
    {
        m_scene->deselect_all();
    }

    if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
    {
        popupFlags |= EditorPopupFlags_SceneAddOption;
    }
}

void Editor::dropGameObjectInHierarchy(Scene *scene)
{
    if (!ImGui::BeginDragDropTarget())
    {
        return;
    }

    auto payload = ImGui::AcceptDragDropPayload(m_extensionDragDrop[DragDropFlags::GAMEOBJECT]);
    if (!payload || payload->DataSize <= 0)
    {
        return ImGui::EndDragDropTarget();
    }

    auto transform = *reinterpret_cast<Transform **>(payload->Data);
    
    transform->setParent(nullptr);
        
    scene->insert(transform->getGameObject());

    ImGui::EndDragDropTarget();
}

void Editor::RenderSceneObject(Scene *scene)
{
    auto list = scene->getObjectList();

    if (m_currentPage > 0 && ImGui::Button("Before"))
    {
        --m_currentPage;
    }

    {
        int start = m_currentPage * ITEMS_PER_PAGE;
        int end = std::min(start + ITEMS_PER_PAGE, static_cast<int>(list.size()));

        for (int i = start; i < end; ++i)
        {
            auto transform = list[i]->getTransform();

            ReadHierarchy(scene, transform);
        }
    }
    
    if (list.size() > ITEMS_PER_PAGE && ImGui::Button("Next"))
    {
        ++m_currentPage;
    }
}

void Editor::ReadHierarchy(Scene *scene, Transform *transform)
{
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanFullWidth;

    auto gameObject = transform->getGameObject();
    if (!gameObject)
    {
        return;
    }

    auto name = gameObject->getName();

    if (scene->object_has_been_selected(gameObject))
        flags |= ImGuiTreeNodeFlags_Selected;

    if (transform->hasChildren())
    {
        flags &= ~ImGuiTreeNodeFlags_Leaf;
    }

    auto isOpen = ImGui::TreeNodeEx(name.c_str(), flags);


    if (ImGui::IsKeyDown(ImGuiKey_ModShift) && ImGui::IsItemClicked(ImGuiMouseButton_Left))
    {
        if (scene->object_has_been_selected(gameObject))
        {
            scene->deselect(gameObject);
        } else
        {
            scene->select(gameObject);
        }
    } else if (ImGui::IsItemClicked(ImGuiMouseButton_Left) && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
    {
        scene->deselect_all();

        scene->select(gameObject);
    }

    dragDropGameObject(transform, name);
    
    if (!isOpen)
    {
        return;
    }
    
    for (auto& child : transform->getChildren())
    {
        ReadHierarchy(scene, child);
    }
    ImGui::TreePop();
}

void Editor::dragDropGameObject(Transform *transform, const std::string &name)
{
    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceNoDisableHover | ImGuiDragDropFlags_SourceNoHoldToOpenOthers))
    {
        ImGui::Text("Moving \"%s\"", name.c_str());
        ImGui::SetDragDropPayload(m_extensionDragDrop[DragDropFlags::GAMEOBJECT], &transform, sizeof(Transform *));
        ImGui::EndDragDropSource();
    }

    if (!ImGui::BeginDragDropTarget())
    {
        return;
    }

    auto payload = ImGui::AcceptDragDropPayload(m_extensionDragDrop[DragDropFlags::GAMEOBJECT]);
    if (!payload || payload->DataSize <= 0)
    {
        return ImGui::EndDragDropTarget();
    }

    auto target = *reinterpret_cast<Transform **>(payload->Data);
    if (target != transform->getParent())
    {
        target->setParent(transform);

        m_scene->remove(target->getGameObject());
    }

    ImGui::EndDragDropTarget();
}

void Editor::PopupMenuAdd()
{
    auto isOpen = ImGui::BeginPopup(GET_FLAG_NAME(EditorPopupFlags_SceneAddOption));
    if (!isOpen)
    {
        return;
    }

    const char* item_name[2] {"Cube", "Empty"};
    auto id = 0;

    ImGui::Text("Add Object");
    if (ImGui::ListBox("##popup_menu_menu_add_list_box", &id, item_name, (sizeof(item_name) / sizeof(item_name[0])))) {
        switch (id) {
        case 0:
            break;
        case 1:
            PostLoadGameObject(GameObject::Create("Empty"));
            break;
        default:
            break;
        }
        ImGui::CloseCurrentPopup();
    }

    ImGui::EndPopup();
}

void Editor::create_window_viewport()
{
    if (ImGui::Begin("viewport"))
    {
        create_render_viewport();
    }
    ImGui::End();
}

void Editor::create_render_viewport()
{
    if (!ImGui::BeginTabBar("##create_render_viewport_tabbar_0"))
    {
        return;
    }

    for (auto &scene : SceneManager::GetScenes())
    {
        auto name = SceneManager::GetSceneName(scene);

        bool bactive = true;
        if (ImGui::BeginTabItem(name, &bactive))
        {
            if (scene != m_scene)
            {
                if (m_scene != nullptr)
                    m_scene->deselect_all();

                m_scene = scene;
            }

            ImGui::EndTabItem();
        }

        if (!bactive)
        {
            if (scene == m_scene)
            {
                m_scene = nullptr;
                SceneManager::SetActive(nullptr);
            }
            SceneManager::Remove(scene);
        }
    }

    if (ImGui::TabItemButton("+"))
    {
        popupFlags |= EditorPopupFlags_CreateScene;
    }

    ImGui::EndTabBar();
    
    drop_scene_file_in_viewport();

    if (m_scene != nullptr)
        render_scene(m_scene);
}

void Editor::drop_scene_file_in_viewport()
{
    auto isOpen = ImGui::BeginDragDropTarget();
    if (!isOpen)
    {
        return;
    }

    auto payload = ImGui::AcceptDragDropPayload(m_extensionDragDrop[DragDropFlags::SCN]);
    if (payload == nullptr)
    {
        return ImGui::EndDragDropTarget();
    }

    AssetsManager::LoadScene(std::string((char*)payload->Data, payload->DataSize), &Editor::set_scene, this);

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

void Editor::render_scene(Scene *scene)
{
    UpdateResizeSceneFramebuffer();

    auto regionAvail = ImGui::GetContentRegionAvail();
    auto windowPos = ImGui::GetWindowPos();
    auto cursorPos = ImGui::GetCursorPos();

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

        if (ImGui::ImageButton(reinterpret_cast<ImTextureID>(s_texturePlayGame->getTextureID()), ImVec2(16, 16))) {
            if (!Settings::bGameRunning) {
                PlayGame();
            } else {
                StopGame();
            }
        }
    }

    // auto texture = Engine::GetImGuiPostprocess()->getTextureColor();
    auto camera = get_camera();
    if (camera != nullptr)
    {
        auto &rendertarget = camera->rendertarget;

        ImGui::Image(reinterpret_cast<ImTextureID>(rendertarget.getTextureID()), regionAvail, {0.0f, 1.0f}, {1.0f, 0.0f});

        /*if (ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
            auto mousePos = ImGui::GetMousePos();
            mousePos -= windowPos + cursorPos;

            mousePos.y = regionAvail.y - mousePos.y;

            int mouseX = static_cast<int>(mousePos.x);
            int mouseY = static_cast<int>(mousePos.y);

            if (mouseX >= 0 && mouseY >= 0 && mouseX <= static_cast<int>(regionAvail.x) && mouseY <= static_cast<int>(regionAvail.y)) {
                int entityID = -1;
                Engine::GetMainPostprocess()->readPixel(mouseX, mouseY, &entityID);

                if (entityID != -1) {
                    auto entity = Manager::GetEntityFromID(static_cast<EntityID>(entityID));
                    if (entity != nullptr)
                        s_objectSelected = entity->getGameObject();
                }
            }
        }
        */
        DropGameObjectInScene();
    }

    ImGuizmo::SetDrawlist();
    ImGuizmo::SetRect(windowPos.x + cursorPos.x, windowPos.y + cursorPos.y, regionAvail.x, regionAvail.y);

    if (m_mainCamera != nullptr && scene->has_object_selected() && !Settings::bGameRunning)
    {
        create_render_guizmo(scene);
    }

    // Fps Draw
    {
        const char *fps_text = "FPS: 000.00f";

        auto textSize = ImGui::CalcTextSize(fps_text);

        ImGui::SetCursorPos(ImVec2(regionAvail.x - textSize.x, cursorPos.y + textSize.y));

        ImGui::TextColored(FPSColor, "FPS: %.2f", (1.0f / Clock::deltaTime));
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

    ImGui::SameLine();
    if (ImGui::RadioButton("Move", EditorController::bCanMove))
    {
        EditorController::bCanMove = !EditorController::bCanMove;
    }
    ImGui::EndGroup();

    ImGui::PopStyleColor();

    if (m_scene->has_object_selected() && ImGui::IsWindowFocused() && mImGuiIO->KeyShift && ImGui::IsKeyReleased(ImGuiKey_D))
    {
        auto temp = m_scene->get_object_selected();
        m_scene->deselect_all();

        for (auto go : temp)
            m_scene->select(GameObject::Instantiate(go));
    }

    if (ImGui::IsWindowFocused() && mImGuiIO->KeyCtrl && ImGui::IsKeyReleased(ImGuiKey_S))
    {
        for (auto &scene : SceneManager::GetScenes())
            TaskManager::Add<SceneSaveTask>(File::GetAsset("/scene"), scene);
    }
}

void Editor::UpdateResizeSceneFramebuffer()
{
    auto camera = get_camera();

    ImVec2 regionAvail = ImGui::GetContentRegionAvail();
    if (m_mainCamera != nullptr && (m_lastViewportSize.x != regionAvail.x || m_lastViewportSize.y != regionAvail.y && regionAvail.x > 5.0f && regionAvail.y > 5.0f))
    {
        auto &viewport = Settings::ViewportSize = {static_cast<int>(regionAvail.x), static_cast<int>(regionAvail.y)};
        if (viewport.x > 0 && viewport.y > 0)
        {
            Display::SetDisplayRect(Rect{0.0f, 0.0f, (float)Settings::ViewportSize.x, (float)Settings::ViewportSize.y});

            m_mainCamera->setAspectRatio(static_cast<float>(viewport.x) / static_cast<float>(viewport.y));

            // Engine::GetMainPostprocess()->resize(viewport);
            
            m_lastViewportSize = regionAvail;
        }
    }
}

void Editor::create_render_guizmo(Scene *scene)
{
    if (ImGui::IsWindowHovered() && !mImGuiIO->KeyShift && !mImGuiIO->KeyCtrl)
    {
        if (ImGui::IsKeyReleased(ImGuiKey_G))
        {
            m_guizmoOperation = ImGuizmo::TRANSLATE;
        }
    
        if (ImGui::IsKeyReleased(ImGuiKey_R))
        {
            m_guizmoOperation = ImGuizmo::ROTATE;
        }
    
        if (ImGui::IsKeyReleased(ImGuiKey_S))
        {
            m_guizmoOperation = ImGuizmo::SCALE;
        }
    }

    if (mImGuiIO->KeyCtrl && ImGui::IsKeyReleased(ImGuiKey_Space) && ImGui::IsWindowHovered()) {
        if (m_guizmoManipulate == ImGuizmo::WORLD) {
            m_guizmoManipulate = ImGuizmo::LOCAL;
        } else if (m_guizmoManipulate == ImGuizmo::LOCAL) {
            m_guizmoManipulate = ImGuizmo::WORLD;
        }
    }

    auto viewmatrix = m_mainCamera->getViewmatrix();
    auto projection = m_mainCamera->getPerspesctive();

    // ImGuizmo::DrawGrid(viewmatrix.getData(), projection.getData(), Matrix4x4::identityMatrix.getData(), 100.f);
    //
    auto objects = scene->get_object_selected();
    for (std::size_t i=0;i<objects.size();i++)
    {
        auto go = objects[i];

        ImGuizmo::SetID(i);
        auto transform = go->getTransform();

        auto globalMatrix = transform->getGlobalMatrix();


        if (ImGuizmo::Manipulate(viewmatrix.getData(), projection.getData(), m_guizmoOperation, m_guizmoManipulate, globalMatrix.getData()))
        {
            auto localMatrix = globalMatrix;
            if (transform->hasParent())
            {
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
}

void Editor::PlayGame()
{
    Threadpool::AddTask(&Editor::BuildScript, this);
}

void Editor::StopGame()
{
    if (m_scene != nullptr)
    {
        m_scene->deselect_all();
    }

    Settings::bGameRunning = false;

    m_scene->stop();

    m_mainCamera->setActive(true);

    Console::log(Log::DEFAULT, "Game stoped.");
}

void Editor::DropGameObjectInScene()
{
    auto isOpen = ImGui::BeginDragDropTarget();
    if (!isOpen)
    {
        return;
    }

    auto payload = ImGui::AcceptDragDropPayload(m_extensionDragDrop[DragDropFlags::GMDX]);
    if (!payload)
    {
        return ImGui::EndDragDropTarget();
    }

    std::string path(reinterpret_cast<char *>(payload->Data), payload->DataSize);
    if (path.empty())
    {
        return ImGui::EndDragDropTarget();
    }
    
    AssetsManager::LoadGameObject(path, &Editor::PostLoadGameObject, this);

    ImGui::EndDragDropTarget();
}

void Editor::OpenFileDoubleClick(std::filesystem::path path) {
    auto extension = path.extension().string();

    if (extension != ".hpp" && extension != ".cpp" && extension != ".glsl") {
        return;
    }

    system(("xdg-open '" + path.string() + "'").c_str());
}

void Editor::PopupFileAndDirmanager()
{
    auto isOpen = ImGui::BeginPopup(GET_FLAG_NAME(EditorPopupFlags_FileManager | EditorPopupFlags_DirManager));
    if (!isOpen) 
    {
        return;
    }

    if (g_tree_file_selected.size() > 1)
    {
        if (ImGui::BeginListBox("list_items"))
        {
            for (auto &tree_file : g_tree_file_selected)
            {
                ImGui::Text("%s", tree_file->name.c_str());
            }
            ImGui::EndListBox();
        }
    } else {
        ImGui::Text("%s", g_tree_file_selected.back()->name.c_str());
    }

    static const char *item_list[] {"Delete"};

    int i = 0;
    if (ImGui::ListBox("##DirManager", &i, item_list, GET_N_ARRAY(item_list)))
    {
        switch (i)
        {
        case 0:
            for (auto &tree_file : g_tree_file_selected)
                File::Delete(tree_file->path);

            g_tree_file_selected.clear();
            break;
        default:
            break;
        }
        updateFileTree(root_tree_file);
        
        ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
}

void Editor::PopupFileManager()
{
    bool isOpenPopup = ImGui::BeginPopup(GET_FLAG_NAME(EditorPopupFlags_FileManager));
    if (!isOpenPopup)
    {
        return;
    }

    if (g_tree_file_selected.size() > 1)
    {
        if (ImGui::BeginListBox("list_items"))
        {
            for (auto &tree_file : g_tree_file_selected)
            {
                ImGui::Text("%s", tree_file->name.c_str());
            }
            ImGui::EndListBox();
        }
    } else {
        ImGui::Text("%s", g_tree_file_selected.back()->name.c_str());
    }

    const char *item_list[] {"Delete", "Rename", "Move", "Copy"};

    int i = 0;
    if (ImGui::ListBox("##FileManager", &i, item_list, GET_N_ARRAY(item_list))) {
        switch (i) {
        case 0:
            {
                for (auto tree_file : g_tree_file_selected)
                    File::Delete(tree_file->path);

            g_tree_file_selected.clear();

            updateFileTree(root_tree_file);
            break;
        }
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

void Editor::PopupDirManager()
{
    bool isOpenPopup = ImGui::BeginPopup(GET_FLAG_NAME(EditorPopupFlags_DirManager));
    if (g_tree_file_selected.empty() || !isOpenPopup)
    {
        return;
    }

    auto &file_tree = g_tree_file_selected.back();

    ImGui::Text("%s", file_tree->name.c_str());

    static const char *item_list[] {"Delete", "Rename", "Create Dir", "Create Script", "Create Material", "Create Shader", "Move", "Copy"};

    int i = 0;
    if (ImGui::ListBox("##FileManager", &i, item_list, GET_N_ARRAY(item_list))) {
        switch (i) {
        case 0:
        {
            File::Delete(file_tree->path);
            g_tree_file_selected.clear();

            updateFileTree(root_tree_file);
            break;
        }
        case 1:
            popupFlags |= EditorPopupFlags_RenameFileOrDir;
            break;
        case 2:
            popupFlags |= EditorPopupFlags_CreateNewDir;
            break;
        case 3:
            popupFlags |= EditorPopupFlags_CreateScript;
            break;
        case 5:
            popupFlags |= EditorPopupFlags_CreateShader;
            break;
        default:
            break;
        }
        ImGui::CloseCurrentPopup();
    }

    ImGui::EndPopup();
}

void Editor::PopupCreateDirectory()
{
    auto isOpen = ImGui::BeginPopup(GET_FLAG_NAME(EditorPopupFlags_CreateNewDir));
    if (!isOpen)
    {
        return;
    }

    auto &file_tree = g_tree_file_selected.back();

    ImGui::Text("Input directory name:");
    if (!ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked(0))
        ImGui::SetKeyboardFocusHere(0);

    char name[256] = {0};
    if (ImGui::InputText("##popup_create_directoty_inputtext", name, 256, ImGuiInputTextFlags_EnterReturnsTrue))
    {
        File::CreateDirectory(file_tree->path + "/" + std::string(name));

        updateFileTree(*file_tree);

        ImGui::CloseCurrentPopup();
    }

    ImGui::EndPopup();
}

void Editor::PopupCreateShader()
{
    auto isOpen = ImGui::BeginPopup(GET_FLAG_NAME(EditorPopupFlags_CreateShader));
    if (!isOpen)
    {
        return;
    }
    ImGui::SeparatorText("Create Shader");

    auto path = mDirSelectedList.back();

    char name[256] = {0};
    if (ImGui::InputText("##input_text_popup_create_shader_name", name, 256, ImGuiInputTextFlags_EnterReturnsTrue))
    {
        CreateShaderTemplate(path.string(), name);

        mDirSelectedList.clear();

        ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
}

void Editor::create_window_browser()
{
    if (ImGui::Begin("browser"))
    {
        create_render_browser();
    }
    ImGui::End();
}

void Editor::create_render_browser()
{
    auto avail = ImGui::GetContentRegionAvail();

    ImGui::SetNextItemOpen(true, ImGuiCond_FirstUseEver);
    if (ImGui::BeginChild("aaa", ImVec2(avail.x / 3.0f, 0), ImGuiChildFlags_AlwaysUseWindowPadding))
    {
        RenderTreeNodeFile(root_tree_file);
    }
    ImGui::EndChild();

    ImGui::SameLine();
    if (ImGui::BeginChild("bbb", ImVec2(0.0f, 0.0f), ImGuiChildFlags_AlwaysUseWindowPadding) && g_tree_file != nullptr)
    {
        RenderFiles();
    }
    ImGui::EndChild();
}

void Editor::RenderTreeNodeFile(gtreefile &tree_file)
{
    if (!tree_file.bdirectory)
    {
        return;
    }

    ImGuiTreeNodeFlags flags = 0;
    if (g_tree_file == &tree_file)
    {
        flags |= ImGuiTreeNodeFlags_Selected;
    }

    auto isOpen = ImGui::TreeNodeEx(tree_file.name.c_str());
    dropTargetGameObjectInFile(tree_file);

    if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
    {
        g_tree_file = &tree_file;
    }

    if (!isOpen)
    {
        return;
    }

    for (auto &file : tree_file.children)
        RenderTreeNodeFile(file);

    ImGui::TreePop();
}

void Editor::RenderFiles()
{
    ImGui::Columns(4, nullptr, false);

    for (auto &entry : g_tree_file->children)
    {
        if (entry.bdirectory || entry.icon == nullptr || *entry.icon == nullptr)
        {
            continue;
        }

        if (View::ImageAndTextButton(reinterpret_cast<ImTextureID>((*entry.icon)->getTextureID()), entry.name.c_str()))
        {
        }

        if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
        {
            popupFlags |= EditorPopupFlags_FileManager;
            
            g_tree_file_selected.clear();

            g_tree_file_selected.push_back(&entry);
        }

        dropTargetGameObjectInFile(entry);

        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceNoDisableHover | ImGuiDragDropFlags_SourceNoHoldToOpenOthers | ImGuiDragDropFlags_SourceAllowNullID)) {
            if (m_extensionNameToFlag.find(entry.extension) != m_extensionNameToFlag.end())
            {
                ImGui::Text("%s", entry.name.c_str());
                ImGui::SetDragDropPayload(m_extensionDragDrop[m_extensionNameToFlag[entry.extension]], entry.path.data(), entry.path.size());
            }
            ImGui::EndDragDropSource();
        }

        if (ImGui::IsItemClicked(ImGuiMouseButton_Left) && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
        {
            OpenFileDoubleClick(entry.path);
        }
        
        ImGui::NextColumn();
    }

    if (!(popupFlags & EditorPopupFlags_FileManager) && ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
    {
        g_tree_file_selected.push_back(g_tree_file);
        popupFlags |= EditorPopupFlags_DirManager;
    }
}

void Editor::dropTargetGameObjectInFile(gtreefile &tree_file)
{
    if (!ImGui::BeginDragDropTarget())
    {
        return;
    }
    
    auto payload = ImGui::AcceptDragDropPayload(m_extensionDragDrop[DragDropFlags::GAMEOBJECT]);
    if (!payload || payload->DataSize <= 0)
    {
        return ImGui::EndDragDropTarget();
    }

    auto transform = *reinterpret_cast<Transform **>(payload->Data);

    TaskManager::Add<GameObjectSaveTask>(tree_file.path, transform->getGameObject(), [&](GameObject *)
            {
                updateFileTree(tree_file);
            });

    ImGui::EndDragDropTarget();
}

void Editor::create_window_console()
{
    if (ImGui::Begin("Console"))
    {
        create_render_console();
    }
    
    ImGui::End();
}

void Editor::create_render_console()
{
    auto logs = Console::GetListLogs();

    std::for_each(logs.begin(), logs.end(), std::bind(&Editor::RenderLog, this, std::placeholders::_1));
    
    if (Console::bChanged)
    {
        ImGui::SetScrollHereY(1.0f);
        Console::bChanged = false;
    }
}

void Editor::RenderLog(Log &info)
{
    ImVec4 color = {1.0f, 1.0f, 1.0f, 1.0f};
    switch (info.type)
    {
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

void Editor::create_window_property()
{
    if (ImGui::Begin("property") && m_scene != nullptr)
    {
        create_render_property();
    }
    ImGui::End();
}

void Editor::create_render_property()
{
    if (!m_scene->has_object_selected())
    {
        return;
    }

    auto go = m_scene->get_object_selected().back();

    auto entity = go->getEntity();

    auto objName = go->getName();

    ImGui::SeparatorText(objName.c_str());

    auto isstatic = go->getStatic();
    if (ImGui::Checkbox("Static", &isstatic))
        go->setStatic(isstatic);

    RenderScriptFromEntity(entity);

    auto avail = ImGui::GetContentRegionAvail();

    if (ImGui::Button("AddComponent", {avail.x, 0.0f}))
    {
        popupFlags |= EditorPopupFlags_AddComponent;
    }

    // dropScriptInAddComponent();

    // ImGui::PopID();
}

void Editor::create_call_popup()
{
    switch (popupFlags)
    {
    case EditorPopupFlags_LoadPlugin:
        ImGui::OpenPopup(GET_FLAG_NAME(EditorPopupFlags_LoadPlugin));
        popupFlags &= ~EditorPopupFlags_LoadPlugin;
        break;
    case EditorPopupFlags_AddComponent:
        ImGui::OpenPopup(GET_FLAG_NAME(EditorPopupFlags_AddComponent));
        popupFlags &= ~EditorPopupFlags_AddComponent;
        break;
    case EditorPopupFlags_PluginEditor:
        ImGui::OpenPopup("##popup_plugin_editor");
        popupFlags &= ~EditorPopupFlags_PluginEditor;
        break;
    case EditorPopupFlags_CreateMaterial:
        ImGui::OpenPopup(GET_FLAG_NAME(EditorPopupFlags_CreateMaterial));
        popupFlags &= ~EditorPopupFlags_CreateMaterial;
        break;
    case EditorPopupFlags_FileManager | EditorPopupFlags_DirManager:
        ImGui::OpenPopup(GET_FLAG_NAME(EditorPopupFlags_FileManager | EditorPopupFlags_DirManager));
        popupFlags &= ~(EditorPopupFlags_FileManager | EditorPopupFlags_DirManager);
        break;
    case EditorPopupFlags_FileManager:
        ImGui::OpenPopup(GET_FLAG_NAME(EditorPopupFlags_FileManager));
        popupFlags &= ~EditorPopupFlags_FileManager;
        break;
    case EditorPopupFlags_DirManager:
        ImGui::OpenPopup(GET_FLAG_NAME(EditorPopupFlags_DirManager));
        popupFlags &= ~EditorPopupFlags_DirManager;
        break;
    case EditorPopupFlags_RenameFileOrDir:
        ImGui::OpenPopup(GET_FLAG_NAME(EditorPopupFlags_RenameFileOrDir));
        break;
    case EditorPopupFlags_CreateNewDir:
        ImGui::OpenPopup(GET_FLAG_NAME(EditorPopupFlags_CreateNewDir));
        break;
    case EditorPopupFlags_CreateScript:
        ImGui::OpenPopup(GET_FLAG_NAME(EditorPopupFlags_CreateScript));
        popupFlags &= ~EditorPopupFlags_CreateScript;
        break;
    case EditorPopupFlags_CreateShader:
        ImGui::OpenPopup(GET_FLAG_NAME(EditorPopupFlags_CreateShader));
        popupFlags &= ~EditorPopupFlags_CreateShader;
        break;
    case EditorPopupFlags_RenameGameObject:
        ImGui::OpenPopup(GET_FLAG_NAME(EditorPopupFlags_RenameGameObject));
        break;
    case EditorPopupFlags_SceneAddOption:
        ImGui::OpenPopup(GET_FLAG_NAME(EditorPopupFlags_SceneAddOption));
        popupFlags &= ~EditorPopupFlags_SceneAddOption;
        break;
    case EditorPopupFlags_CreateScene:
        ImGui::OpenPopup(GET_FLAG_NAME(EditorPopupFlags_CreateScene));
    default:
        break;
    }

    popupFlags = 0;
}

void Editor::create_popup()
{
    create_popup_create_scene();

    create_popup_add_component();

    create_popup_rename_object();

    create_popup_create_script();

    create_popup_rename_filedir();

    PopupComponentMenu();

    SkyboxEditor();

    PluginManager();

    PopupPluginLoad();

    PluginEditor();

    RenameMaterial();

    CreateMaterial();

    PopupFileAndDirmanager();

    PopupFileManager();

    PopupDirManager();

    PopupCreateDirectory();

    PopupCreateShader();

    PopupMenuAdd();
}

void Editor::create_popup_create_scene()
{
    auto isOpen = ImGui::BeginPopup(GET_FLAG_NAME(EditorPopupFlags_CreateScene));
    if (!isOpen)
    {
        return;
    }

    ImGui::Text("Input scene name");

    if (!ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked(ImGuiMouseButton_Left))
        ImGui::SetKeyboardFocusHere();

    char name[256] = {0};
    if (ImGui::InputText("##popup_create_scene_inputtext", name, GET_N_ARRAY(name), ImGuiInputTextFlags_EnterReturnsTrue))
    {
        m_scene = SceneManager::Create(name);
        m_scene->init();
        m_scene->create_main_camra();

        set_scene(m_scene);

        ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
}

void Editor::create_popup_add_component()
{
    auto isOpen = ImGui::BeginPopup(GET_FLAG_NAME(EditorPopupFlags_AddComponent));
    if (!isOpen)
    {
        return;
    }

    auto go = m_scene->get_object_selected().back();

    std::vector<std::string> m_componentName;
    for (auto &componentInfo : ComponentRegistry::GetComponentInfoList())
    {
        m_componentName.push_back(componentInfo->name);
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
        if (ImGui::Selectable(it->c_str(), isFirst) || bSelect && go != nullptr)
        {
            auto component = go->getEntity()->addComponent((*it));

            std::memset(componentName, '\0', sizeof(componentName));

            ImGui::CloseCurrentPopup();
            
            break;
        }
    }
    
    ImGui::EndPopup();
}

void Editor::create_popup_rename_object()
{
    auto isopen = ImGui::BeginPopup(GET_FLAG_NAME(EditorPopupFlags_RenameGameObject));
    if (!isopen)
    {
        return;
    }

    ImGui::SeparatorText("[ Rename ]");
    auto objectlist = m_scene->get_object_selected();

    auto object = objectlist.back();
    auto go_name = object->getName();

    char name[256] = {0};
    strcpy(name, go_name.c_str());

    if (!ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked(ImGuiMouseButton_Left))
        ImGui::SetKeyboardFocusHere();

    if (ImGui::InputText("##create_popup_rename_object_inputtext_0", name, GET_N_ARRAY(name), ImGuiInputTextFlags_EnterReturnsTrue))
    {
        object->setName(name);

        ImGui::CloseCurrentPopup();
    }

    ImGui::EndPopup();
}

void Editor::create_popup_create_script()
{
    auto isOpen = ImGui::BeginPopup(GET_FLAG_NAME(EditorPopupFlags_CreateScript));
    if (!isOpen)
    {
        return;
    }

    auto &file_tree = g_tree_file_selected.back();

    ImGui::Text("Input your script name");

    char name[256] = {0};
    if (ImGui::InputText("##input_text_popup_create_script_name", name, 256, ImGuiInputTextFlags_EnterReturnsTrue))
    {
        CreateScriptTemplate(file_tree->path, name);

        updateFileTree(*file_tree);

        ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
}

void Editor::create_popup_rename_filedir()
{
    auto isOpen = ImGui::BeginPopup(GET_FLAG_NAME(EditorPopupFlags_RenameFileOrDir));
    if (!isOpen)
    {
        return;
    }

    auto &tree_file = g_tree_file_selected.back();

    ImGui::Text("Rename:");
    if (!ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked(ImGuiMouseButton_Left))
        ImGui::SetKeyboardFocusHere(0);

    auto filename = tree_file->name;
    char name[256] = {0};
    strcpy(name, filename.c_str());

    if (ImGui::InputText("##popup_rename_file", name, GET_N_ARRAY(name), ImGuiInputTextFlags_EnterReturnsTrue))
    {
        File::rename(tree_file->path, tree_file->parent->path + "/" + name);

        updateFileTree(*tree_file->parent);
        
        ImGui::CloseCurrentPopup();
    }

    ImGui::EndPopup();
}

#include <gr-base/ecs/gameobjectmanager.hpp>
#include <gr-base/core/status.hpp>

void Editor::RenderStatisticsMenu() {
    ImGui::SeparatorText("Scripts");
    ImGui::Text("Entidades: %ld", EntityManager::GetCount());
    ImGui::Text("GameObject: %ld", GameObjectManager::GetCount());

    ImGui::SeparatorText("AssetManager");
    ImGui::Text("Textures: %ld", AssetsManager::GetTextureCount());
    // ImGui::Text("Shaders: %ld", AssetsManager::GetShaderCount());
    ImGui::Text("Mesh: %ld", AssetsManager::GetMeshCount());
    // ImGui::Text("Event: %ld", Event::GetCount());

    ImGui::SeparatorText("Threads");
    ImGui::Text("Threads: %ld/%ld", Threadpool::GetThreadCount(), Threadpool::GetCount());

    ImGui::SeparatorText("FrameTime");
    {
        auto &frameTime = Statistics::FrameTime[FrameTimeID_MainThread];
        
        ImGui::Text("MainThread:\t%fms ->(%fms, %fms), FPS: %.2f ->(%f, %f)", frameTime.delta, frameTime.minDelta, frameTime.maxDelta, frameTime.fps, frameTime.minFPS, frameTime.maxFPS);
    }
    /*ImGui::Text("Render:      %fms  | FPS: %.2f",  Statistics::m_sceneFrame.delta,   Statistics::m_sceneFrame.fps);*/
    /*ImGui::Text("GUI:         %fms  | FPS: %.2f",  Statistics::m_guiFrame.delta,     Statistics::m_guiFrame.fps);*/
    /*ImGui::Text("Script:      %fms  | FPS: %.2f",  Statistics::m_scriptFrame.delta,  Statistics::m_scriptFrame.fps);*/

    ImGui::SeparatorText("Renderização");
    ImGui::Text("Luzes: %ld",     Light::GetListLight().size());
    ImGui::Text("Cameras: %ld",     Camera::GetCameras().size());
    /*ImGui::Text("BatchRender: %ld/%ld", Batching::GetBatchCount(), Status::BatchRenderCount);*/
    ImGui::Text("DrawCallCount: %ld/%ld", Status::BatchDrawCallCount, Status::BatchDrawCallRender);
}

void Editor::dropScriptInAddComponent()
{
    /*if (s_objectSelected == nullptr || !ImGui::BeginDragDropTarget())
    {
        return;
    }
    
    auto data = ImGui::AcceptDragDropPayload(m_extensionDragDrop[DragDropFlags::SCRIPT], ImGuiDragDropFlags_None);
    if (!data)
    {
        return ImGui::EndDragDropTarget();
    }

    std::filesystem::path file = std::string((char*)data->Data, data->DataSize);

    auto entity = s_objectSelected->getEntity();
    if (entity != nullptr)
    {
        entity->addComponent(file.filename().replace_extension().string());
    }
    ImGui::EndDragDropTarget();
    */
}

void Editor::RenderScriptFromEntity(Entity *entity)
{
    for (auto [name, component] : entity->getComponentMap())
    {
        ImGui::PushID(name.c_str());

        auto it = mInternalScript.find(name);
        if (it != mInternalScript.end())
        {
            it->second(entity);
        } else if (!RenderComponents(name, &(*component)))
        {
            entity->destroy(name);
        }

        ImGui::PopID();
    }
}

bool Editor::RenderComponents(const std::string &name, Component *component)
{
    auto isRender = true;

    auto isOpen = ImGui::CollapsingHeader(name.c_str(), &isRender);
    if (!isRender)
    {
        return false;
    }

    if (!isOpen)
    {
        return true;
    }

    auto baseChunk = ComponentRegistry::Find(name);
    if (baseChunk == nullptr)
    {
        Console::LogError("Invalid component: " + name);
        return false;
    }

    auto fieldList = baseChunk->getFields();

    for (auto &field : fieldList)
    {
        RenderField(field, component);
    }

    return true;
}

void Editor::RenderField(Field &field, Component *component) {
    auto name = field.getName();

    ImGui::Text("%s:", name.c_str());
    name = "##" + name;
    
    switch (field.getType()) {
    case FieldType_GameObject: {
        renderDropGameObject(Field::GetFieldData<GameObject *>(component, field));
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
    
    auto data = ImGui::AcceptDragDropPayload(m_extensionDragDrop[DragDropFlags::GMDX], ImGuiDragDropFlags_None);
    if (!data) {
        return ImGui::EndDragDropTarget();
    }
    
    auto path = std::string((char*)data->Data, data->DataSize);

    AssetsManager::LoadGameObject(path, &Editor::OutputGameObject, ref);

    ImGui::EndDragDropTarget();
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
    if (View::DragScalarEx(data, (float *)&position, 3, 1.0f, 0.0f, 0.0f))
    {
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

bool Editor::OnPropertyCamera(Camera *camera)
{
    bool bactive = true;
    auto isOpen = ImGui::CollapsingHeader("Camera", &bactive);
    if (!bactive)
    {
        return true;
    }

    if (!isOpen)
    {
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

    if (isChanged)
    {
        camera->setRect(rect);
    }

    if (ImGui::CollapsingHeader("Render"))
    {
        if (camera->get_aspect_ratio() != (64.0f / 64.0f))
            camera->setAspectRatio(64.0f / 64.0f);

        auto &rendertarget = camera->rendertarget;

        if (rendertarget.is_valid())
            ImGui::Image(reinterpret_cast<ImTextureID>(rendertarget.getTextureID()), {64.0f, 64.0f}, {0.0f, 1.0f}, {1.0f, 0.0f});
    }

    return false;
}

bool Editor::OnPropertyMeshFilter(MeshFilter *meshFilter)
{
    auto bExists = true;

    auto isOpen = ImGui::CollapsingHeader("MeshFilter", &bExists);
    if (!bExists)
    {
        return true;
    }

    auto meshList = meshFilter->getMesh();

    if (!isOpen || meshList == nullptr)
    {
        return false;
    }

    auto name = AssetsManager::GetMeshListRefToName(meshList);

    if (ImGui::TreeNode(name.c_str()))
    {
        for (std::size_t i=0;i<meshList->size();i++)
        {
            auto &mesh = (*meshList)[i];

            ImGui::Text("ID: %ld Name: %s", i, mesh.getName().c_str());
        }
        ImGui::TreePop();
    }
    return false;
}

bool Editor::OnPropertyMeshRender(MeshRender *meshRender)
{
    auto bVisible = true;

    auto isOpen = ImGui::CollapsingHeader("MeshRender", &bVisible);
    if (!bVisible)
    {
        return true;
    }
    
    if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
        ImGui::OpenPopup("##game_object_component_menu");
    }

    if (!isOpen)
    {
        return false;
    }
   
    auto avail = ImGui::GetContentRegionAvail();
    
    if (ImGui::Button("Create", {avail.x, 0.0f}))
    {
        popupFlags |= EditorPopupFlags_CreateMaterial;
    }

    ImGui::Indent();
    for (auto material : meshRender->getMaterial())
    {
        if (render_material(material))
        {
            meshRender->delMaterial(material);
        }
    }
    ImGui::Unindent();

    if (m_materialToRename)
    {
        ImGui::OpenPopup("##popup_rename_material");
    }

    return false;
}

bool Editor::OnPropertySkinnedMeshRender(SkinnedMeshRender *skinnedMeshRender) {
    auto bVisible = true;

    auto isOpen = ImGui::CollapsingHeader("SkinnedMeshRender", &bVisible);
    if (!bVisible)
    {
        return true;
    }
    
    if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
        ImGui::OpenPopup("##game_object_component_menu");
    }

    if (!isOpen) {
        return false;
    }
   
    auto avail = ImGui::GetContentRegionAvail();
    
    if (ImGui::Button("Create", {avail.x, 0.0f})) {
        popupFlags |= EditorPopupFlags_CreateMaterial;
    }

    ImGui::Indent();
    for (auto material : skinnedMeshRender->getMaterial())
    {
        if (render_material(material))
        {
            skinnedMeshRender->delMaterial(material);
        }
    }
    ImGui::Unindent();

    if (m_materialToRename) {
        ImGui::OpenPopup("##popup_rename_material");
    }

    return false;
}

bool Editor::render_material(Material *material)
{
    auto bVisible = true;

    auto name = AssetsManager::GetRefMaterialToName(material);

    auto isOpen = ImGui::CollapsingHeader(name.c_str(), &bVisible);
    if (!bVisible)
    {
        return true;
    }
    
    if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
    {
        m_materialToRename = material;
    }

    if (!isOpen)
    {
        return false;
    }

    auto shader = material->getShader();
    if (shader == nullptr)
    {
        dropShaderInMaterial(material);
        return false;
    }

    auto shaderParams = GetShaderParamsFromName(AssetsManager::GetShaderName(shader));

    auto shadername = std::filesystem::path(shaderParams.base).filename();

    render_shader_property(shadername, material);

    render_material_property(material);

    dropShaderInMaterial(material);

    return false;
}

void Editor::render_shader_property(const std::string &name, Material *material)
{
    auto isopen = ImGui::TreeNode(name.c_str());
    if (!isopen)
        return;

    RenderShaderUniform(material, material->getShader());

    ImGui::TreePop();
}

void Editor::render_material_property(Material *material)
{
    auto isopen = ImGui::TreeNode("Property");
    if (!isopen)
        return;

    auto tilling = material->get_scalling();
    auto offset = material->get_offset();

    ImGui::Text("Tilling");
    if (ImGui::DragFloat2("##render_material_property_tilling", (float *)&tilling, 1.0f, 1.1f))
    {
        material->set_scaling(tilling);
    }
    
    ImGui::Text("Offset");
    if (ImGui::DragFloat2("##render_material_property_offset", (float *)&offset, 1.0f, 1.0f))
    {
        material->set_offset(offset);
    }

    ImGui::TreePop();
}

void Editor::RenderShaderUniform(Material *material, grr::gShader *shader)
{
    auto property = AssetsManager::GetShaderProperty(shader);
    if (property == nullptr)
    {
        return;
    }

    for (auto &&[object, name, type] : property->getProperty())
    {
        switch (type)
        {
        case UniformVariable::MAT3:
        case UniformVariable::MAT4:
        case UniformVariable::NONE:
            continue;
        default:
            break;
        }

        ImGui::SeparatorText(name.c_str());

        switch (type)
        {
            case UniformVariable::VEC3:
            {
                auto value = material->getValue<Vector3>(name);
                if (value == nullptr)
                    value = material->setValue<Vector3>(name, Vector3::zero);

                if (value != nullptr)
                    ImGui::DragFloat3(("##" + std::string(name)).c_str(), (float *)value);
                break;
            }
            case UniformVariable::FLOAT:
            {
                auto value = material->getValue<float>(name);
                if (value == nullptr)
                    value = material->setValue<float>(name, 0.0f);

                if (value != nullptr)
                    ImGui::DragFloat(("##" + std::string(name)).c_str(), (float *)value);
                break;
            }
            /*case UniformVariable::int: {
            int value = 0.0f;
            if (uniform.data != nullptr)
                value = *reinterpret_cast<int *>(uniform.data);

            if (ImGui::DragInt(("##" + std::string(uniform.name)).c_str(), &value))
            {
                shader->setUniform(uniform.name, &value);
            }
            break;
        }
        case UniformVariable::float: {
            float value = 0.0f;
            if (uniform.data != nullptr)
                value = *reinterpret_cast<float *>(uniform.data);

            if (ImGui::DragFloat(("##" + std::string(uniform.name)).c_str(), &value))
            {
                shader->setUniform(uniform.name, &value);
            }
            break;
        }
        case UniformVariable::vec3: {
            Vector3 value = Vector3::zero;
            if (uniform.data != nullptr)
                value = *reinterpret_cast<Vector3 *>(uniform.data);

            if (ImGui::DragFloat3(("##" + std::string(uniform.name)).c_str(), (float *)&value))
            {
                shader->setUniform(uniform.name, &value);
            }
            break;
        }
        case UniformVariable::vec4: {
            Vector4 value = Vector4::zero;
            if (uniform.data != nullptr)
                value = *reinterpret_cast<Vector4 *>(uniform.data);

            if (ImGui::DragFloat3(("##" + std::string(uniform.name)).c_str(), (float *)&value))
            {
                shader->setUniform(uniform.name, &value);
            }
            break;
        }
        */
        default:
            break;
        }
        
    }
}

void Editor::dropShaderInMaterial(Material *material) {
    if (!ImGui::BeginDragDropTarget())
    {
        return;
    }

    auto payload = ImGui::AcceptDragDropPayload(m_extensionDragDrop[DragDropFlags::SHADER]);
    if (!payload)
    {
        return ImGui::EndDragDropTarget();
    }
    
    std::string file((char*)payload->Data, payload->DataSize);
    if (file.empty() || file.find(".glsl") == std::string::npos)
    {
        return ImGui::EndDragDropTarget();
    }

    std::cout << "Wrong: " << file << std::endl;

    // AssetsManager::LoadShader(AssetShader_Shader3D, file, &Material::setShader, material);

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

bool Editor::OnPropertyRigidbody(Rigidbody *rigidbody)
{
    auto bExists = true;

    auto isOpen = ImGui::CollapsingHeader("Rigidbody", &bExists);
    if (!bExists)
    {
        return true;
    }
    
    if (!isOpen) {
        return false;
    }

    auto mass = rigidbody->getMass();

    ImGui::Text("Mass");
    if (ImGui::DragFloat("##rigidbody_mass", &mass))
    {
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

    auto animation = animator->getAnimation();
    auto animName = animation != nullptr ? animation->getName() : "No Animation";

    ImGui::Text("Animation: %s", animName.c_str());

    dropAnimation(animator);

    ImGui::DragFloat("Factor", &animator->blendFactor, 0.01f, 0.0f, 1.0f);
    
    return false;
}

void Editor::dropAnimation(Animator *animator)
{
    if (!ImGui::BeginDragDropTarget())
    {
        return;
    }

    auto payload = ImGui::AcceptDragDropPayload(m_extensionDragDrop[DragDropFlags::ANIMATION]);
    if (!payload || payload->DataSize <= 0) {
        return ImGui::EndDragDropTarget();
    }
    
    std::string path((char*)payload->Data, payload->DataSize);
    
    AssetsManager::LoadAnimation(path, &Animator::play, animator, true);
    
    ImGui::EndDragDropTarget();
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
    AssetsManager::LoadTexture(File::GetAsset("/icon/icons8-script-48.png", true),                  &Editor::SetupTexture, this, IconType::ICON_SCRIPT);
}

void Editor::SetupTexture(IconType type, grr::gTexture *texture) {
    iconsUI[type] = texture;

    if (type == IconType::ICON_PLAY_GAME) {
        s_texturePlayGame = iconsUI[IconType::ICON_PLAY_GAME];
    }
}

Camera *Editor::getCamera()
{
    return m_mainCamera;
}

void Editor::PostLoadGameObject(GameObject *gameObject)
{
    if (m_scene == nullptr)
    {
        return;
    }

    auto instantiate = GameObject::Instantiate(gameObject);
    GameObject::Destroy(gameObject);

    m_scene->deselect_all();
    m_scene->select(instantiate);
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

void Editor::CreateShaderTemplate(const std::string &path, const std::string &name)
{
    std::ofstream os(path + "/" + name + ".glsl");
    os << "Sample" << std::endl;
    os.close();
}

// PopupMenu

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

void Editor::CreateMaterial()
{
    /*
    auto isOpen = ImGui::BeginPopupModal(GET_FLAG_NAME(EditorPopupFlags_CreateMaterial));
    if (!isOpen || s_objectSelected == nullptr)
    {
        return;
    }

    ImGui::Text("Name");
    if (!ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked(ImGuiMouseButton_Left))
        ImGui::SetKeyboardFocusHere(0);

    char name[256] = {0};

    if (ImGui::InputText("##popup_create_material_inputtext", name, GET_N_ARRAY(name), ImGuiInputTextFlags_EnterReturnsTrue))
    {
        auto baseRender = s_objectSelected->hasComponent<MeshRender>() ? static_cast<BaseRender *>(s_objectSelected->getComponent<MeshRender>()) : static_cast<BaseRender *>(s_objectSelected->getComponent<SkinnedMeshRender>());
        if (baseRender != nullptr)
            baseRender->createMaterial(name);

        ImGui::CloseCurrentPopup();
    }

    ImGui::EndPopup();
    */
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

void Editor::PostBuildScriptFromProject()
{
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
        MainThreadDispatcher::Add(func);
}

/*
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
*/

void Editor::SkyboxEditor() {
    auto isOpen = ImGui::BeginPopupModal("##popup_skybox_editor", nullptr, ImGuiWindowFlags_Modal);
    if (!isOpen) { return; }

    ImGui::Text("Teste");

    ImGui::EndPopup();
}

#include "plugin/pluginManager.hpp"

void Editor::PluginManager()
{
    ImVec2 winSize(200, 300);
    
    ImGui::SetNextWindowPos(ImVec2{static_cast<float>(Screen::width) / 2.0f, static_cast<float>(Screen::height) / 2.0f} - (winSize / 2.0f), ImGuiCond_Always);
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

void Editor::PopupPluginLoad() {
    ImVec2 winSize(200, 300);
    
    ImGui::SetNextWindowPos(ImVec2{static_cast<float>(Screen::width) / 2.0f, static_cast<float>(Screen::height) / 2.0f} - (winSize / 2.0f), ImGuiCond_Always);
    ImGui::SetNextWindowSize(winSize, ImGuiCond_Always);
    auto isOpen = ImGui::BeginPopup(GET_FLAG_NAME(EditorPopupFlags_LoadPlugin));
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
    
    ImGui::SetNextWindowPos(ImVec2{static_cast<float>(Screen::width) / 2.0f, static_cast<float>(Screen::height) / 2.0f} - (winSize / 2.0f), ImGuiCond_Always);
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

void Editor::set_scene(Scene *scene)
{
    m_cameraObject = scene == nullptr ? nullptr : scene->getCameraObject();

    m_mainCamera = scene == nullptr ? nullptr : scene->getCamera();

    m_scene = scene;
}

void Editor::OutputGameObject(GameObject **ref, GameObject *go)
{
    if ((*ref) != nullptr)
        GameObject::Destroy(*ref);

    *ref = go;
}

gConfigStep_ Editor::foreach_settings(gConfigStep_ type, void *data, void *instance)
{
    auto editor = static_cast<Editor *>(instance);

    switch (type)
    {
        case gConfigType_Section:
        {
            auto section = static_cast<struct gSection *>(data);
            if (std::strcmp(section->name, "Scenes") == 0)
            {
                return gConfigStep_Recuse;
            }
            break;
        }
        case gConfigType_Subsection:
        {
            auto section = static_cast<struct gSection *>(data);
            if (std::strcmp(section->name, "Scene") == 0)
            {
                auto name = config_get_string(section, "name");

                AssetsManager::LoadScene(File::GetAsset("/scene/") + name + ".scn", &Editor::set_scene, editor);
            }
            break;
        }
        default:
            break;
    }
    return gConfigStep_Continue;
}

void Editor::generateConfig(const std::string &file)
{
    auto scenes = SceneManager::GetScenes();
    if (!scenes.empty())
    {
        auto section = config_find_section(m_settings, "Scenes");
        if (section != nullptr)
        {
            config_remove_section(m_settings, section);
        }

        section = config_create_section(m_settings, "Scenes", nullptr);
        
        for (auto &scene : scenes)
        {
            auto name = SceneManager::GetSceneName(scene);

            auto subsection = config_create_sub_section(section, "Scene", nullptr);
            config_set_string(subsection, "name", name);
        }
    }

    {
        auto section = config_find_section(m_settings, "Script");
        if (section != nullptr)
        {
            config_remove_section(m_settings, section);
        }

        section = config_create_section(m_settings, "Script", nullptr);

        if (config_get_string(section, "libname") == nullptr)
        {
            config_set_string(section, "libname", "libgr-component.so");
        }
        
        if (config_get_string(section, "dir") == nullptr)
        {
            config_set_string(section, "dir", "/.build/component/lib/");
        }
    }
}

void Editor::updateFileTree(gtreefile &tree_file)
{
    if (!tree_file.bdirectory)
    {
        return;
    }

    auto &children = tree_file.children;

    std::vector<std::filesystem::directory_entry> files;
    for (auto &dir : std::filesystem::directory_iterator(tree_file.path))
    {
        files.push_back(dir);
    }

    std::sort(files.begin(), files.end(), [](auto& lhs, auto& rhs) {
        if (lhs.is_directory() != rhs.is_directory()) {
            return lhs.is_directory() > rhs.is_directory();
        }
        return lhs.path().filename() < rhs.path().filename();
    });

    children.clear();

    children.resize(files.size());
    for (std::size_t i=0;i<files.size();i++)
    {
        auto &child = children[i];

        child.name          = files[i].path().filename().string();
        child.path          = files[i].path().string();
        child.parent        = &tree_file;
        child.extension     = files[i].path().extension();
        child.bdirectory    = files[i].is_directory();

        auto extension = files[i].path().extension().string();

        auto texture = &iconsUI[IconType::ICON_FILE];
        if (extension == ".gmdx")
        {
            texture = &iconsUI[IconType::ICON_GAMEOBJECT];
        } else if (extension == ".anm")
        {
            texture = &iconsUI[IconType::ICON_ANIMATION];
        } else if (extension == ".scn")
        {
            texture = &iconsUI[IconType::ICON_SCENE];
        } else if (extension == ".glsl")
        {
            texture = &iconsUI[IconType::ICON_SHADER];
        } else if (extension == ".hpp" || extension == ".cpp")
        {
            texture = &iconsUI[IconType::ICON_SCRIPT];
        } else if (extension == ".png")
        {
            texture = &iconsUI[IconType::ICON_TEXTURE];
        }

        child.icon = texture;

        updateFileTree(child);
    }
}

Camera *Editor::get_camera()
{
    return Settings::bGameRunning ? Camera::Current() : m_mainCamera;
}

PhysicsTask::PhysicsTask() : IAsyncTask<bool>(TaskPriority::HIGH)
{}

bool PhysicsTask::doInBackground()
{
    Time time;

    while (Settings::bGameRunning)
    {
        auto dt = time.sleep(60.0f);

        PhysicsScene::Update(dt);
    }

    return true;
}


