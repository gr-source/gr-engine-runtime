#include "gr-engine/view/editor.hpp"

#include <atomic>
#include <chrono>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <gr-base/script/baserender.hpp>
#include <gr-base/script/default.hpp>

#include "gr-base/managers/bvhtreemanager.hpp"
#include "gr-base/managers/chunkmanager.hpp"
#include <gr-base/managers/eventManager.hpp>

#include "gr-engine/Global.h"
#include "gr-engine/core/time.hpp"
#include "gr-engine/editorController.h"
#include "gr-engine/engine.h"

#include "gr-base/core/clock.hpp"
#include "gr-base/core/debugdraw.hpp"
#include "gr-base/core/geometry/shader.hpp"
#include "gr-base/core/mainthreaddispatcher.hpp"
#include "gr-base/core/assetReference.hpp"

#include "gr-base/core/task/runtime/gameobjectsavetask.hpp"
#include "gr-base/core/task/runtime/scenesavetask.hpp"
#include "gr-base/core/thread/asynctask.hpp"
#include "gr-base/core/thread/taskmanager.hpp"

#include "gr-base/ecs/behaviorStorage.hpp"

#include "gr-base/physics/physicsScene.hpp"

#include "gr-engine/managers/viewmanager.hpp"
#include "gr-engine/runtime/editorRuntime.hpp"
#include "gr-engine/settings.h"

#include "gr-engine/ui/viewlayout.hpp"
#include "imgui.h"

#include <gr-base/core/window.hpp>

#include <gr-base/core/file/file.hpp>

#include <gr-base/core/thread/threadpool.hpp>

#include <gr-base/managers/gameobjectmanager.hpp>
#include <gr-base/managers/assetsManager.hpp>
#include <gr-base/managers/rendermanager.hpp>
#include <gr-base/managers/octreemanager.hpp>
#include <gr-base/managers/sceneManager.hpp>

#include <gr-base/core/console.hpp>
#include <gr-base/core/display.hpp>
#include <gr-base/core/scene.hpp>

#include "gr-engine/plugin/pluginManager.hpp"

#include "gr-engine/core/screen.hpp"

#include <gr-base/script/skinnedMeshRender.hpp>
#include <gr-base/script/LODComponent.hpp>
#include <gr-base/script/meshCollider.hpp>
#include <gr-base/script/boxCollider.hpp>
#include <gr-base/script/meshFilter.hpp>
#include <gr-base/script/meshRender.hpp>
#include <gr-base/script/transform.hpp>
#include <gr-base/script/rigidbody.hpp>
// #include <gr-base/script/animator.hpp>
#include <gr-base/script/camera.hpp>
#include <gr-base/script/light.hpp>

#include <gr-base/core/gameObject.hpp>

#include "gr-engine/guizmo/ImGuizmo.h"

#include "gr-engine/ui/ImGuiRender.h"
#include "gr-engine/ui/project.hpp"
#include "gr-engine/ui/view.hpp"

#include "gr-engine/scriptManager/scriptManager.hpp"

#include <gr-base/core/geometry/batching.hpp>

// #include <gr-base/broadphase/bvhtree.hpp>

#include <gr-ini/config.h>
#include <gr-math/gmath.hpp>
#include <gr-math/matrix4x4.hpp>
#include <gr-render/gTexture.h>
#include <filesystem>
#include <iostream>
#include <string>

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
    MESH        = 1 << 8,
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
    {DragDropFlags::MESH,       ".gmsh"},
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
    {".gmsh",       DragDropFlags::MESH},
};

#define INSERT_SCRIPT(object) \
    {#object, [this](Entity *entity) \
        { \
            RenderProperty<object>(&Editor::OnProperty##object, this); \
        } \
    }


#include <gr-base/managers/batchingmanager.hpp>

class TestPopup : public ViewLayout
{
    int numInstances;

    Vector2 position;

    std::vector<GameObjectHandle> m_instances;

public:
    TestPopup() : numInstances(100), position({0.0f, 0.0f})
    {}

    const char* getName() const override
    {
        return "TestPopup";
    }

    void onRenderUI() override
    {
        ImGui::Begin("Test");

        ImGui::InputInt("Num", &numInstances);

        if (ImGui::Button("Spawn"))
        {
            if (!m_instances.empty())
            {
                for (auto& handle : m_instances)
                    GameObject::DestroyImediate(handle);

                m_instances.clear();
            }

            std::string file = AssetsManager::GetAssetDir() + "/model/Cubo.gmdx";

            AssetsManager::LoadGameObject(file, &TestPopup::SetPrefab, this);
        }

        ImGui::SameLine();

        if (ImGui::Button("Despawn"))
        {
            for (auto& handle : m_instances)
                GameObject::DestroyImediate(handle);

            m_instances.clear();
        }

        ImGui::End();
    }

    void SetPrefab(GameObjectHandle prefabID)
    {
        Spawn(prefabID);
    }

    void Spawn(GameObjectHandle prefabID)
    {
        int distance = numInstances / 2;

        int currentX = floor(position.x / 16.0f);
        int currentZ = floor(position.y / 16.0f);

        for (int x=-distance;x<distance;x++)
        {
            for (int z=-distance;z<distance;z++)
            {
                int targetX = (currentX + x);
                int targetZ = (currentZ + z);

                auto prefab = GameObject::Get(prefabID);

                auto gameobjectID = GameObject::Instantiate(prefab);

                auto gameObject = GameObject::Get(gameobjectID);
                gameObject->setName("Test[" + std::to_string(targetX) + "][" + std::to_string(targetZ) + "]");

                auto transform = gameObject->GetTransform();
                transform->setLocalPosition({(float)targetX * 5.5f, 0.0f, (float)targetZ * 4.5f});

                m_instances.push_back(gameobjectID);
            }
        }

        position += Vector2{(float)distance * 16.0f * 2.0f, 0.0f};

        GameObject::DestroyImediate(prefabID);

        // MainThreadDispatcher::AddDelayed(10.0f, &TestPopup::Spawn, this, prefabID);
    }
};

Editor::Editor()
    : m_viewport(ImGui::GetMainViewport()),
    mImGuiIO(&ImGui::GetIO()),
    m_currentPage(0),
    m_materialToRename(ID_INVALID),
    m_plugin(nullptr),
    m_state(EngineState::Edit),
    m_runtime(nullptr),
    m_playMode(nullptr)
{
    m_cameraObjectSelected = nullptr;

    m_scene = nullptr;

    s_texturePlayGame = nullptr;

    g_treefile_dir_selected = nullptr;

    m_currentPage = 0;

    popupFlags = 0;

    FPSColor = {1.0f, 1.0f, 1.0f, 1.0f};

    m_guizmoOperation = ImGuizmo::TRANSLATE;

    m_guizmoManipulate = ImGuizmo::WORLD;

    Engine& engine = Engine::GetInstance();

    m_project = &engine.get_project();

    m_guiscriptmap = {
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
        INSERT_SCRIPT(LODComponent),
        INSERT_SCRIPT(MeshCollider),
        INSERT_SCRIPT(BoxCollider),
        INSERT_SCRIPT(Rigidbody),
        // INSERT_SCRIPT(Animator),
    };
    
    m_viewport = ImGui::GetMainViewport();

    m_editMode = std::make_unique<EditorRuntime>();

    m_runtime = m_editMode.get();
    m_runtime->initialize();

    // Initialize
    {
        AssetReference::Initialize();

        AssetsManager::Initialize();

        BatchingManager::Initialize();
        
        DebugDraw::Initialize();
    }

    AssetsManager::SetAssetDir(m_project->path + "/assets");

    AssetReference::Load(m_project->path + "/asset.txt");

    {
        GameObjectHandle gameObjectID = GameObject::Create("Camera");

        GameObject* gameObject = GameObject::Get(gameObjectID);
        gameObject->addComponent<Camera>();

        m_cameraID = gameObject->GetID<Camera>();
    }

    bBuildDock = gfile::exists("imgui.ini");

    LoadReources();

    try
    {
        auto libdir = m_project->path + "/.build/lib/libscript.so";
        if (gfile::exists(libdir))
            ScriptManager::Load(libdir);

        Display::SetDisplayRect(Rect{0.0f, 0.0f, (float)Settings::ViewportSize.x, (float)Settings::ViewportSize.y});
    } catch (const std::exception &e)
    {
        Console::LogError("Fatal error in script: %s", e.what());
    }

    root_tree_file.path = AssetsManager::GetAssetDir();
    root_tree_file.name = "Assets";
    root_tree_file.parent = nullptr;
    root_tree_file.bdirectory = true;
    root_tree_file.icon = nullptr;

    updateFileTree(&root_tree_file);

    Window::SetWindowTitle(String::format("GR - %s", m_project->name.c_str()).c_str());

    ViewManager::AddPopup<TestPopup>();

    EventManager::Attach("delete_file_selected", &Editor::delete_file_selected, this);
    EventManager::Attach("editor_update_tree_file", &Editor::updateFileTree, this, &root_tree_file);
}

Editor::~Editor()
{
    m_playMode = nullptr;
    m_editMode = nullptr;

    AssetReference::Save(m_project->path + "/asset.txt");

    DebugDraw::Shutdown();

    BatchingManager::Shutdown();

    AssetReference::Shutdown();

    AssetsManager::Shutdown();
}

void Editor::loadScript(const std::string& dir)
{
    ScriptManager::Unload();
    
    if (ScriptManager::CheckScriptHasInMemory("libscript.so"))
        return Console::LogError("[ Script ] Fatal error data not unloaded.");

    ScriptManager::Load(dir + "/libscript.so");

    // BehaviorStorage::Reinstantiate();

    Display::SetDisplayRect(Rect{0.0f, 0.0f, (float)Settings::ViewportSize.x, (float)Settings::ViewportSize.y});

    std::string temp = gfile::get_cache_directory() + "/snapshot.scn";

    AssetsManager::SceneSaveAsync(temp, m_scene, &Editor::OnPostSaveSnapshot, this, temp);
}

// ===================================== END =========================================================
const char* Editor::getName() const
{
    return "Editor";
}

#include <gr-base/core/renderSystem.hpp>
void Editor::onUpdate()
{
    auto scene = SceneManager::GetCurrent();
    if (Settings::bGameRunning)
    {
        BehaviorStorage::Update();
    } else
    {
        if (scene != nullptr && scene->getCamera())
            EditorController::Update(scene->getCamera());
    }

    ChunkManager::Update();

    // if (scene != nullptr)
        // scene->Update();

    GameObjectManager::Cleanup();

    RenderSystem::Update();
}

void Editor::onRender3D()
{
    // OctreeManager::Render();

    // BVHTreeManager::Render();

    // ChunkManager::Render();

    if (!Settings::bGameRunning)
    {
        auto scene = SceneManager::GetCurrent();
        if (scene != nullptr && scene->getCamera())
            on_pre_config_render_scene(scene->getCamera());

        if (m_cameraObjectSelected != nullptr)
            on_pre_config_render_scene(m_cameraObjectSelected);
    } else if (m_scene != nullptr)
    {
        for (auto &cameraID : m_scene->GetCameras())
            on_pre_config_render_scene(BehaviorStorage::GetComponent<Camera>(cameraID));
    }

    DebugDraw::Clear();
}

void Editor::on_pre_config_render_scene(Camera* camera)
{
    RenderManager::PreRender(camera);

    auto rendertarget = camera->getRenderTarget();
    rendertarget->bind();

    grr::gRender::SetRenderState(grr::GR_CULL, grr::GR_BACK);
    grr::gRender::SetEnable(GR_DEPTH, true);

    grr::gRender::SetBackgroundColor(camera->background);
    grr::gRender::SetRenderState(grr::GR_BACKGROUND, grr::GR_DEPTH_BUFFER | grr::GR_COLOR_BUFFER);

    on_render_scene(camera);

    rendertarget->unbind();
}

void Editor::on_render_scene(Camera* camera)
{
    grr::gRender::SetRenderState(grr::GR_DEPTH_FUNC, grr::GR_DEPTH_LESS);
    RenderSystem::Renderer();
    // RenderManager::Render();

    grr::gRender::SetRenderState(grr::GR_DEPTH_FUNC, grr::GR_DEPTH_LEQUAL);
    DebugDraw::Render(camera);

    // grr::gRender::SetRenderState(grr::GR_DEPTH_FUNC, grr::GR_DEPTH_LEQUAL);
    // m_skybox.render(camera);
}

void Editor::onRenderUI()
{
    ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());

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
    updateFileTree(&root_tree_file);
}

void Editor::create_dock()
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("Project"))
        {
            if (ImGui::MenuItem("Close"))
                MainThreadDispatcher::Add(Priority::LOW, &Engine::init_project, &Engine::GetInstance(), "");
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Scene"))
        {
            if (ImGui::MenuItem("Bake"))
            {
                for (auto& gameObjectID : m_scene->getObjectList())
                {
                    auto gameobject = GameObject::Get(gameObjectID);

                    auto transform = gameobject->GetTransform();
                    auto position = transform->getPosition();

                    int chunkIdX = floor(position.x / CHUNK_SIZE);
                    int chunkIdZ = floor(position.z / CHUNK_SIZE);

                    std::string key = std::to_string(chunkIdX) + "_" + std::to_string(chunkIdZ);

                    ChunkID chunkID = ChunkManager::Find(key);
                    if (chunkID == INVALID_CHUNK)
                    {
                        chunkID = ChunkManager::Create(key);

                        StreamChunk* chunk = ChunkManager::Get(chunkID);
                        chunk->position = Vector2{(float)chunkIdX, (float)chunkIdZ};
                        chunk->objects = nullptr;
                        chunk->dirty = false;
                    }

                    StreamChunk* chunk = ChunkManager::Get(chunkID);
                    chunk->dirty = true;
                    vector_push(chunk->objects, gameObjectID);
                }
            }
            ImGui::EndMenu();
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

void Editor::create_window_hierarchy()
{
    if (ImGui::Begin("hierarchy"))
        create_render_hierarchy();

    ImGui::End();
}

void Editor::create_render_hierarchy()
{
    std::string name = m_scene != nullptr ? SceneManager::GetSceneName(m_scene) : "Scene name is empty!";

    ImGui::SetNextItemOpen(true, ImGuiCond_Appearing);
    if (m_scene != nullptr && ImGui::TreeNodeEx(name.c_str(), ImGuiTreeNodeFlags_SpanFullWidth))
    {
        drop_gameobject([this](GameObject *gameobject)
                {
                    Transform *transform = gameobject->GetTransform();
                    transform->setParent(nullptr);

                    m_scene->insert(transform->GetGameObject()->getCacheID());
                });
        
        RenderSceneObject(m_scene);

        ImGui::TreePop();
    }

    if (ImGui::IsKeyReleased(ImGuiKey_Delete) && m_scene->has_object_selected())
    {
        for (auto go : m_scene->get_object_selected())
            m_scene->destroy(go);

        m_scene->deselect_all();

        auto objects = m_scene->getObjectList();
        if (!objects.empty())
            m_scene->select(objects.back());
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
        popupFlags |= EditorPopupFlags_SceneAddOption;
}

void Editor::drop_gameobject(std::function<void (GameObject *)> func)
{
    if (!ImGui::BeginDragDropTarget())
        return;

    auto payload = ImGui::AcceptDragDropPayload(m_extensionDragDrop[DragDropFlags::GAMEOBJECT]);
    if (!payload || payload->DataSize <= 0)
        return ImGui::EndDragDropTarget();

    auto transform = *reinterpret_cast<Transform **>(payload->Data);
    
    func(transform->GetGameObject());

    ImGui::EndDragDropTarget();
}

void Editor::RenderSceneObject(Scene *scene)
{
    auto list = scene->getObjectList();

    if (m_currentPage > 0 && ImGui::Button("Before"))
        --m_currentPage;

    {
        int start = m_currentPage * ITEMS_PER_PAGE;
        int end = std::min(start + ITEMS_PER_PAGE, static_cast<int>(list.size()));

        for (int i = start; i < end; ++i)
        {
            auto gameObject = GameObject::Get(list[i]);
            if (gameObject != NULL)
                ReadHierarchy(scene, gameObject);
        }
    }
    
    if (list.size() > ITEMS_PER_PAGE && ImGui::Button("Next"))
        ++m_currentPage;
}

void Editor::ReadHierarchy(Scene *scene, GameObject* gameObject)
{
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanFullWidth;

    auto transform = gameObject->GetTransform();
    auto name = gameObject->getName();
    bool isstatic = gameObject->GetIsStatic();

    if (scene->object_has_been_selected(gameObject->getCacheID()))
        flags |= ImGuiTreeNodeFlags_Selected;

    if (transform->hasChildren())
        flags &= ~ImGuiTreeNodeFlags_Leaf;

    ImGui::PushID(transform);

    auto isOpen = ImGui::TreeNodeEx(name.c_str(), flags);

    if (ImGui::IsKeyDown(ImGuiKey_ModShift) && ImGui::IsItemClicked(ImGuiMouseButton_Left))
    {
        if (scene->object_has_been_selected(gameObject->getCacheID()))
        {
            scene->deselect(gameObject->getCacheID());
        } else
        {
            scene->select(gameObject->getCacheID());
        }
    } else if (ImGui::IsItemClicked(ImGuiMouseButton_Left) && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
    {
        scene->deselect_all();

        scene->select(gameObject->getCacheID());
    }

    dragDropGameObject(transform, name);
    
    if (isOpen)
    {
        auto children = transform->getChildren();

        for (auto& child : children)
            ReadHierarchy(scene, BehaviorStorage::GetComponent<Transform>(child)->GetGameObject());

        ImGui::TreePop();
    }

    ImGui::PopID();
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
        return;

    auto payload = ImGui::AcceptDragDropPayload(m_extensionDragDrop[DragDropFlags::GAMEOBJECT]);
    if (!payload || payload->DataSize <= 0)
        return ImGui::EndDragDropTarget();

    auto target = *reinterpret_cast<Transform **>(payload->Data);
    if (target != transform->getParent())
    {
        target->setParent(transform);

        m_scene->remove(target->GetGameObject()->getCacheID());
    }

    ImGui::EndDragDropTarget();
}

void Editor::PopupMenuAdd()
{
    auto isOpen = ImGui::BeginPopup(GET_FLAG_NAME(EditorPopupFlags_SceneAddOption));
    if (!isOpen)
        return;

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
        create_render_viewport();
    ImGui::End();
}

void Editor::create_render_viewport()
{
    if (!ImGui::BeginTabBar("##create_render_viewport_tabbar_0"))
        return;

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
        popupFlags |= EditorPopupFlags_CreateScene;

    ImGui::EndTabBar();
    
    drop_scene_file_in_viewport();

    if (m_scene != nullptr)
        render_scene(m_scene);
}

void Editor::drop_scene_file_in_viewport()
{
    auto isOpen = ImGui::BeginDragDropTarget();
    if (!isOpen)
        return;

    auto payload = ImGui::AcceptDragDropPayload(m_extensionDragDrop[DragDropFlags::SCN]);
    if (payload == nullptr)
        return ImGui::EndDragDropTarget();

    std::vector<gtreefile *> files(payload->DataSize / sizeof(gtreefile *), nullptr);
    memcpy(files.data(), payload->Data, payload->DataSize);

    for (auto file : files)
        AssetsManager::LoadScene(file->path, &Editor::set_scene, this);

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

    EditorController::bCanMove = ImGui::IsWindowHovered();

    auto regionAvail = ImGui::GetContentRegionAvail();
    auto windowPos = ImGui::GetWindowPos();
    auto cursorPos = ImGui::GetCursorPos();

    // Play button
    if (s_texturePlayGame) {
        s_texturePlayGame = iconsUI[ICON_PLAY_GAME];
        if (Settings::bGameRunning) {
            if (ImGui::IsKeyReleased(ImGuiKey_Escape) && ImGui::IsWindowHovered()) {
                OnStopButton();
            } else {
                s_texturePlayGame = iconsUI[ICON_STOP_GAME];
            }
        }
        
        ImGui::SetCursorPos({regionAvail.x / 2 - cursorPos.x, cursorPos.y + 0.5f});

        if (ImGui::ImageButton(reinterpret_cast<ImTextureID>(s_texturePlayGame->getTextureID()), ImVec2(16, 16))) {
            if (!Settings::bGameRunning)
            {
                OnPlayButton();
            } else {
                OnStopButton();
            }
        }
    }

    // auto texture = Engine::GetImGuiPostprocess()->getTextureColor();
    auto camera = m_scene->getCamera();
    if (camera != nullptr)
    {
        regionAvail = ImGui::GetContentRegionAvail();
        cursorPos = ImGui::GetCursorPos();

        auto rendertarget = camera->getRenderTarget();

        ImGui::Image(reinterpret_cast<ImTextureID>(rendertarget->GetTextureID()), regionAvail, {0.0f, 1.0f}, {1.0f, 0.0f});

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
        drop_gameobject_in_scene();
    }

    ImGuizmo::SetDrawlist();
    ImGuizmo::SetRect(windowPos.x + cursorPos.x, windowPos.y + cursorPos.y, regionAvail.x, regionAvail.y);

    if (camera != nullptr && scene->has_object_selected() && !Settings::bGameRunning)
        create_render_guizmo(scene);

    // Fps Draw
    {
        char fps_text[256] = {0};
        sprintf(fps_text, "FPS: %.2f", (1.0f / Clock::deltaTime));

        auto textSize = ImGui::CalcTextSize(fps_text);

        ImGui::SetCursorPos(ImVec2(regionAvail.x - textSize.x, cursorPos.y + textSize.y));

        ImGui::TextColored(FPSColor, "%s", fps_text);
    }

    ImGui::SetCursorPos({8.0f, cursorPos.y});

    ImGui::PushStyleColor(ImGuiCol_Text, {0.0f, 0.0f, 1.0f, 1.0f});

    ImGui::BeginGroup();
    if (ImGui::RadioButton("Local", m_guizmoManipulate == ImGuizmo::LOCAL))
        m_guizmoManipulate = ImGuizmo::LOCAL;
    
    ImGui::SameLine();
    if (ImGui::RadioButton("World", m_guizmoManipulate == ImGuizmo::WORLD))
        m_guizmoManipulate = ImGuizmo::WORLD;

    ImGui::EndGroup();

    ImGui::PopStyleColor();

    if (m_scene->has_object_selected() && ImGui::IsWindowFocused() && mImGuiIO->KeyShift && ImGui::IsKeyReleased(ImGuiKey_D))
    {
        auto temp = m_scene->get_object_selected();
        m_scene->deselect_all();

        for (auto go : temp)
            m_scene->select(GameObject::Instantiate(GameObject::Get(go)));
    }

    if (ImGui::IsWindowFocused() && mImGuiIO->KeyCtrl && ImGui::IsKeyReleased(ImGuiKey_S))
    {
        std::string path = m_project->path + "/assets/scene";

        for (auto &scene : SceneManager::GetScenes())
            TaskManager::Add<int, SceneSaveTask>(path + "/" + SceneManager::GetSceneName(scene) + ".scn", scene);
    }
}

void Editor::UpdateResizeSceneFramebuffer()
{
    auto camera = m_scene->getCamera();
    if (camera == nullptr)
        return;

    auto rect = camera->getRect();

    ImVec2 regionAvail = ImGui::GetContentRegionAvail();
    if (camera != nullptr && (rect.w != regionAvail.x || rect.h != regionAvail.y && regionAvail.x > 5.0f && regionAvail.y > 5.0f))
    {
        auto &viewport = Settings::ViewportSize = {regionAvail.x, regionAvail.y};
        if (viewport.x > 0 && viewport.y > 0)
        {
            Display::SetDisplayRect(Rect{0.0f, 0.0f, (float)Settings::ViewportSize.x, (float)Settings::ViewportSize.y});

            camera->setRect(Rect{0.0f, 0.0f, regionAvail.x, regionAvail.y});
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

    auto camera = m_scene->getCamera();

    auto viewmatrix = camera->getViewmatrix();
    auto projection = camera->getPerspesctive();

    static bool IsMoving = false;
    static std::unordered_set<Transform *> nodeup;
    for (auto gameObjectID : scene->get_object_selected())
    {
        // TODO: Não deveria ser assim por que o objeto ainda permanece na lista ?
        auto go = GameObject::Get(gameObjectID);
        if (go == nullptr)
            continue;

        ImGuizmo::SetID(gameObjectID);
        auto transform = go->GetTransform();

        auto lastPosition = transform->getLocalPosition();

        auto globalMatrix = transform->getGlobalMatrix();

        bool bChanged = ImGuizmo::Manipulate(&viewmatrix[0][0], &projection[0][0], m_guizmoOperation, m_guizmoManipulate, &globalMatrix[0][0]);
        if (bChanged)
        {
            auto localMatrix = globalMatrix;
            if (transform->hasParent())
            {
                auto parentMat = transform->getParent()->getGlobalMatrix();
                Matrix4x4 m = Matrix4x4::identityMatrix;

                if (gluInvertMatrix(&parentMat[0][0], &m[0][0]))
                {
                    localMatrix = m * localMatrix;
                }
            }

            Vector3 s;
            Quaternion r;
            Vector3 t;
            localMatrix.decompose(s, r, t);

            transform->setLocalScale(s);
            transform->setLocalRotation(r);
            transform->setLocalPosition(t);

            nodeup.emplace(transform);

            IsMoving = true;
        }

        static bool isDragging = false;
        if (ImGuizmo::IsUsing() && !isDragging) 
        {
            isDragging = true;
        }
        if (isDragging && !ImGuizmo::IsUsing()) 
        {
            isDragging = false;

            /*
            RenderManager::Remove(gameObjectID);

            RenderManager::Insert(gameObjectID);
            */
        }
    }
}

void Editor::OnPlayButton()
{
    Threadpool::AddTask(&Editor::OnBuildScript, this);
}

#include "gr-engine/core/cmd/cmd.hpp"
void Editor::OnBuildScript()
{
    Console::LogInfo("[ Build ] Starting pre-build scripting...");

    auto builddir = m_project->path + "/.build";
    if (!gfile::exists(builddir))
    {
        if (!gfile::create_directory(builddir))
            return Console::LogError("[ Build ] fail to create directory: %s", builddir.c_str());

        auto templatedir = gfile::get_data_directory() + "/assets/script";

        gfile::copy(templatedir.c_str(), builddir.c_str());
    }

    auto cmakebuild = builddir + "/build";
    if (!gfile::exists(cmakebuild))
    {
        if (!gfile::create_directory(cmakebuild))
            return Console::LogError("[ Build ] fail to create directory: %s", cmakebuild.c_str());

        std::string cmd = "cd " + cmakebuild + " && cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ..";

        bool retVal = Command::exec(cmd, [](std::string line)
        {
            Console::LogWarning(line);
        });

        if (!retVal)
            return Console::LogError("[ Build ] Falied in to configure build");
    }

    std::string cmd = "cmake --build " + cmakebuild;

    bool retVal = Command::exec(cmd, [](std::string line)
    {
        Console::LogWarning(line);
    });

    if (!retVal)
        return Console::LogError("[ Build ] Falied in to build script");

    Console::LogInfo("[ Build ] Script compiled sucess fully.");

    MainThreadDispatcher::Add(Priority::NORMAL, &Editor::loadScript, this, builddir + "/lib");
}

void Editor::OnPostSaveSnapshot(std::string file)
{
    std::cout << "PlayGame" << std::endl;

    m_playMode = std::make_unique<EditorRuntime>();
    m_runtime = m_playMode.get();
    m_runtime->initialize();

    m_scene = nullptr;

    AssetsManager::LoadScene(file, &Editor::OnPostLoadSnapshot, this);
}

void Editor::OnPostLoadSnapshot(Scene* scene)
{
    m_scene = scene;

    m_scene->bRunning = true;

    Settings::bGameRunning = true;

    TaskManager::Add<int, PhysicsTask>();

    BehaviorStorage::Start();

    // set_scene(scene);
}

void Editor::OnStopButton()
{
    MainThreadDispatcher::Add(Priority::HIGHT, [&]
    {

        if (m_scene != nullptr)
            m_scene->deselect_all();

        m_state = EngineState::Edit;

        m_playMode = nullptr;

        m_runtime = m_editMode.get();
        m_runtime->initialize();

        m_scene = SceneManager::GetCurrent();
        m_scene->SetCamera(m_cameraID);

        Settings::bGameRunning = false;

        Console::LogDefault("Game stoped.");
    });
}

void Editor::drop_gameobject_in_scene()
{
    auto isOpen = ImGui::BeginDragDropTarget();
    if (!isOpen)
        return;

    auto payload = ImGui::AcceptDragDropPayload(m_extensionDragDrop[DragDropFlags::GMDX]);
    if (!payload)
        return ImGui::EndDragDropTarget();

    std::vector<gtreefile *> files(payload->DataSize / sizeof(gtreefile *), nullptr);
    memcpy(files.data(), payload->Data, payload->DataSize);

    for (auto &file : files)
    {
        if (file->extension == ".gmdx")
            AssetsManager::LoadGameObject(file->path, &Editor::PostLoadGameObject, this);
    }

    unselect_all_file();

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
                gfile::remove(tree_file->path);

            g_tree_file_selected.clear();
            break;
        default:
            break;
        }
        updateFileTree(&root_tree_file);
        
        ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
}

void Editor::PopupFileManager()
{
    ImGui::SetNextWindowSize({200.0f, 100.0f});
    bool isOpenPopup = ImGui::BeginPopup(GET_FLAG_NAME(EditorPopupFlags_FileManager));
    if (!isOpenPopup)
    {
        return;
    }

    if (View::ButtonWithComment("Remove", "delete"))
    {
        for (auto &file : g_tree_file_selected)
        {
            gfile::remove(file->path);
        }

        unselect_all_file();

        // FIXME: Muito lento
        updateFileTree(&root_tree_file);
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

    ImGui::Text("Input directory name:");
    if (!ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked(0))
        ImGui::SetKeyboardFocusHere(0);

    char name[256] = {0};
    if (ImGui::InputText("##popup_create_directoty_inputtext", name, 256, ImGuiInputTextFlags_EnterReturnsTrue))
    {
        gfile::create_directories(g_treefile_dir_selected->path + "/" + std::string(name));

        updateFileTree(g_treefile_dir_selected);

        ImGui::CloseCurrentPopup();
    }

    ImGui::EndPopup();
}

void Editor::PopupCreateShader()
{
    /*
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
    */
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
        render_node_directory(&root_tree_file);
    }
    ImGui::EndChild();

    ImGui::SameLine();
    if (ImGui::BeginChild("bbb", ImVec2(0.0f, 0.0f), ImGuiChildFlags_AlwaysUseWindowPadding) && g_treefile_dir_selected != nullptr)
    {
        render_files_on_dir(g_treefile_dir_selected);
    }
    ImGui::EndChild();
}

void Editor::render_node_directory(gtreefile *tree_file)
{
    if (!tree_file->bdirectory)
    {
        return;
    }

    ImGuiTreeNodeFlags flags = 0;
    if (g_treefile_dir_selected == tree_file)
        flags |= ImGuiTreeNodeFlags_Selected;

    auto isOpen = ImGui::TreeNodeEx(tree_file->name.c_str(), flags);

    drop_gameobject_in_treefile(tree_file);

    if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
    {
        g_treefile_dir_selected = tree_file;

        popupFlags |= EditorPopupFlags_DirManager;
    } else if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
    {
        g_treefile_dir_selected = tree_file;
    }

    if (!isOpen)
        return;

    for (auto &file : tree_file->children)
        render_node_directory(&file);

    ImGui::TreePop();
}

void Editor::render_files_on_dir(gtreefile *dir)
{
    ImGui::Columns(4, nullptr, false);
    for (auto &entry : dir->children)
    {
        if (entry.bdirectory || entry.icon == nullptr || *entry.icon == nullptr)
        {
            continue;
        }

        if (View::ImageAndTextButton(reinterpret_cast<ImTextureID>((*entry.icon)->getTextureID()), entry.name.c_str(), selected_file(&entry)))
        {
        }

        if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
        {
            if (!selected_file(&entry))
                select_file(&entry);

            popupFlags |= EditorPopupFlags_FileManager;
        } else if (mImGuiIO->KeyShift && ImGui::IsItemClicked(ImGuiMouseButton_Left))
        {
            if (!selected_file(&entry))
                select_file(&entry);
        }

        drop_gameobject_in_treefile(&entry);

        drag_file(&entry);

        if (ImGui::IsItemClicked(ImGuiMouseButton_Left) && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
        {
            OpenFileDoubleClick(entry.path);
        }
        
        ImGui::NextColumn();
    }

    if (ImGui::IsWindowHovered())
    {
        if (mImGuiIO->KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_A, false))
        {
            unselect_all_file();

            for (auto &entry : dir->children)
            {
                if (entry.bdirectory)
                {
                    continue;
                }

                select_file(&entry);
            }
        }
        if (ImGui::IsKeyPressed(ImGuiKey_Escape, false))
        {
            unselect_all_file();
        }
    }

    if (ImGui::IsWindowHovered())
    {
        if (ImGui::IsKeyPressed(ImGuiKey_Delete, false))
        {
            EventManager::Notify("delete_file_selected");
        }
        else if (!(popupFlags & EditorPopupFlags_FileManager) && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
        {
            unselect_all_file();

            select_file(dir);

            popupFlags |= EditorPopupFlags_CurrentDirManager;
        }
    }
}

void Editor::drag_file(gtreefile *file)
{
    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceNoDisableHover | ImGuiDragDropFlags_SourceNoHoldToOpenOthers | ImGuiDragDropFlags_SourceAllowNullID))
    {
        if (m_extensionNameToFlag.find(file->extension) != m_extensionNameToFlag.end())
        {
            if (!selected_file(file))
                select_file(file);

            auto count = g_tree_file_selected.size() > 10 ? 10 : g_tree_file_selected.size();

            auto pos = ImGui::GetCursorPos();
            auto after = pos;

            for (std::size_t i={};i<count;i++)
            {
                int id = i % 2;

                ImGui::SetCursorPos(pos);
                pos.y -= 5.5f;
                pos.x = after.x + (id * 5.0f);

                ImGui::Image(reinterpret_cast<ImTextureID>(iconsUI[IconType::ICON_FILE]->getTextureID()), {32, 32});
            }

            ImGui::SetDragDropPayload(m_extensionDragDrop[m_extensionNameToFlag[file->extension]], g_tree_file_selected.data(), g_tree_file_selected.size() * sizeof(gtreefile *));
        }
        ImGui::EndDragDropSource();
    }
}

void Editor::drop_gameobject_in_treefile(gtreefile *tree_file)
{
    if (!ImGui::BeginDragDropTarget())
        return;
    
    auto payload = ImGui::AcceptDragDropPayload(m_extensionDragDrop[DragDropFlags::GAMEOBJECT]);
    if (!payload || payload->DataSize <= 0)
        return ImGui::EndDragDropTarget();

    auto transform = *reinterpret_cast<Transform **>(payload->Data);

    auto gameobject = transform->GetGameObject();


    TaskManager::Add<int, GameObjectSaveTask>(tree_file->path + "/" + gameobject->getName() + ".gmdx", gameobject->getCacheID(), [&](GameObjectHandle id)
            {
                EventManager::Notify("editor_update_tree_file");
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
    auto logs = Console::GetLogs();

    std::for_each(logs.begin(), logs.end(), std::bind(&Editor::RenderLog, this, std::placeholders::_1));
    
    if (Console::IsChanged())
    {
        ImGui::SetScrollHereY(1.0f);
        Console::StopChange();
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

void Editor::create_window_property()
{
    if (ImGui::Begin("property") && m_scene != nullptr && m_scene->has_object_selected())
        create_render_property();

    ImGui::End();
}

void Editor::create_render_property()
{
    auto go = GameObject::Get(m_scene->get_object_selected().back());
    if (go == nullptr)
        return;

    auto objName = go->getName();

    ImGui::SeparatorText(objName.c_str());

    bool isStatic = go->GetIsStatic();
    if (ImGui::Checkbox("static", &isStatic))
    {
        go->SetIsStatic(isStatic);

        // RenderManager::SwitchTo(go->getCacheID(), isStatic);
    }

    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("Definir como Estático");

    RenderScriptFromEntity(go);

    auto avail = ImGui::GetContentRegionAvail();

    if (ImGui::Button("AddComponent", {avail.x, 0.0f}))
        popupFlags |= EditorPopupFlags_AddComponent;
}

void Editor::create_call_popup()
{
    switch (popupFlags)
    {
    case EditorPopupFlags_LoadPlugin:
        ImGui::OpenPopup(GET_FLAG_NAME(EditorPopupFlags_LoadPlugin));
        break;
    case EditorPopupFlags_AddComponent:
        ImGui::OpenPopup(GET_FLAG_NAME(EditorPopupFlags_AddComponent));
        break;
    case EditorPopupFlags_PluginEditor:
        ImGui::OpenPopup("##popup_plugin_editor");
        break;
    case EditorPopupFlags_CreateMaterial:
        ImGui::OpenPopup(GET_FLAG_NAME(EditorPopupFlags_CreateMaterial));
        break;
        /*
    case EditorPopupFlags_FileManager | EditorPopupFlags_DirManager:
        ImGui::OpenPopup(GET_FLAG_NAME(EditorPopupFlags_FileManager | EditorPopupFlags_DirManager));
        popupFlags &= ~(EditorPopupFlags_FileManager | EditorPopupFlags_DirManager);
        break;
        */
    case EditorPopupFlags_FileManager:
        ImGui::OpenPopup(GET_FLAG_NAME(EditorPopupFlags_FileManager));
        break;
    case EditorPopupFlags_CurrentDirManager:
        ImGui::OpenPopup(GET_FLAG_NAME(EditorPopupFlags_CurrentDirManager));
        break;
    case EditorPopupFlags_RenameFileOrDir:
        ImGui::OpenPopup(GET_FLAG_NAME(EditorPopupFlags_RenameFileOrDir));
        break;
    case EditorPopupFlags_CreateNewDir:
        ImGui::OpenPopup(GET_FLAG_NAME(EditorPopupFlags_CreateNewDir));
        break;
    case EditorPopupFlags_CreateScript:
        ImGui::OpenPopup(GET_FLAG_NAME(EditorPopupFlags_CreateScript));
        break;
    case EditorPopupFlags_CreateShader:
        ImGui::OpenPopup(GET_FLAG_NAME(EditorPopupFlags_CreateShader));
        break;
    case EditorPopupFlags_RenameGameObject:
        ImGui::OpenPopup(GET_FLAG_NAME(EditorPopupFlags_RenameGameObject));
        break;
    case EditorPopupFlags_SceneAddOption:
        ImGui::OpenPopup(GET_FLAG_NAME(EditorPopupFlags_SceneAddOption));
        break;
    case EditorPopupFlags_CreateScene:
        ImGui::OpenPopup(GET_FLAG_NAME(EditorPopupFlags_CreateScene));
        break;
    case EditorPopupFlags_DirManager:
        ImGui::OpenPopup(GET_FLAG_NAME(EditorPopupFlags_DirManager));
        break;
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

    create_popup_info_dir();

    create_popup_dir_manager();

    create_popup_new_material();

    PluginManager();

    PopupPluginLoad();

    PluginEditor();

    RenameMaterial();

    PopupFileAndDirmanager();

    PopupFileManager();

    PopupCreateDirectory();

    PopupCreateShader();

    PopupMenuAdd();
}

void Editor::create_popup_create_scene()
{
    auto isOpen = ImGui::BeginPopup(GET_FLAG_NAME(EditorPopupFlags_CreateScene));
    if (!isOpen)
        return;

    ImGui::Text("Input scene name");

    if (!ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked(ImGuiMouseButton_Left))
        ImGui::SetKeyboardFocusHere();

    char name[256] = {0};
    if (ImGui::InputText("##popup_create_scene_inputtext", name, GET_N_ARRAY(name), ImGuiInputTextFlags_EnterReturnsTrue))
    {
        m_scene = SceneManager::Create(name);
        m_scene->SetCamera(m_cameraID);

        // set_scene(m_scene);

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

    array* hashtable = BehaviorRegistry::GetBehaviorInfos();

    ComponentInfo* infos = (ComponentInfo*)hashtable->vector;
    for (auto hash : BehaviorRegistry::GetActiveBehaviorActive())
    {
        ComponentInfo* info = BehaviorRegistry::Get(hash);

        m_componentName.push_back(info->name);
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
        if (ImGui::Selectable(it->c_str(), isFirst) || bSelect && go != HANDLE_INVALID)
        {
            auto gameObject = GameObject::Get(go);

            auto component = gameObject->addComponent(it->c_str());

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

    auto object = GameObject::Get(objectlist.back());
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
        updateFileTree(file_tree);

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

        updateFileTree(tree_file->parent);
        
        ImGui::CloseCurrentPopup();
    }

    ImGui::EndPopup();
}

void Editor::create_popup_info_dir()
{
    auto isopen = ImGui::BeginPopup(GET_FLAG_NAME(EditorPopupFlags_CurrentDirManager));
    if (!isopen)
        return;

    if (ImGui::Selectable("Create directory"))
        popupFlags |= EditorPopupFlags_CreateNewDir;

    if (ImGui::Selectable("Create script"))
        popupFlags |= EditorPopupFlags_CreateScript;

    if (ImGui::Selectable("Create material"))
        popupFlags |= EditorPopupFlags_CreateMaterial;

    ImGui::EndPopup();
}

void Editor::create_popup_dir_manager()
{
    auto isopen = ImGui::BeginPopup(GET_FLAG_NAME(EditorPopupFlags_DirManager));
    if (!isopen)
        return;

    if (ImGui::Selectable("Create directory"))
    {
        popupFlags |= EditorPopupFlags_CreateNewDir;
    }

    if (ImGui::Selectable("Remove"))
    {
        gfile::remove(g_treefile_dir_selected->path);

        g_treefile_dir_selected = g_treefile_dir_selected->parent;

        updateFileTree(g_treefile_dir_selected);
    }

    ImGui::EndPopup();
}

void Editor::create_popup_new_material()
{
    auto isOpen = ImGui::BeginPopup(GET_FLAG_NAME(EditorPopupFlags_CreateMaterial));
    if (!isOpen)
        return;

    ImGui::SeparatorText("Create Material");

    ImGui::Text("Name");
    if (!ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked(ImGuiMouseButton_Left))
        ImGui::SetKeyboardFocusHere(0);

    char name[256] = {0};

    if (ImGui::InputText("##popup_create_material_inputtext", name, GET_N_ARRAY(name), ImGuiInputTextFlags_EnterReturnsTrue))
    {
        std::string path = m_project->path + "/assets/material/" + name;
        /*
        auto baseRender = s_objectSelected->hasComponent<MeshRender>() ? static_cast<BaseRender *>(s_objectSelected->getComponent<MeshRender>()) : static_cast<BaseRender *>(s_objectSelected->getComponent<SkinnedMeshRender>());
        if (baseRender != nullptr)
            baseRender->createMaterial(name);
        */
        ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
}

#include <gr-base/core/status.hpp>

void Editor::RenderStatisticsMenu()
{
    ImGui::SeparatorText("Scripts");
    ImGui::Text("GameObject: %ld", GameObjectManager::GetCount());

    ImGui::SeparatorText("AssetManager");
    ImGui::Text("Textures: %ld", AssetsManager::GetTextureCount());
    ImGui::Text("Material: %d", AssetsManager::GetMaterialCount());
    ImGui::Text("Mesh: %ld", AssetsManager::GetMeshCount());

    ImGui::SeparatorText("Threads");
    ImGui::Text("Threads: %ld/%ld", Threadpool::GetThreadCount(), Threadpool::GetCount());

    ImGui::SeparatorText("Renderização");
    if (m_scene != nullptr)
    {
        ImGui::Text("Luzes: %ld",       m_scene->GetLights().size());
        ImGui::Text("Cameras: %ld",     m_scene->GetCameras().size());
        ImGui::Text("Objects: %ld",     m_scene->getObjectList().size());
    }
    ImGui::Text("DrawCall Count: %ld", Status::BatchDrawCallCount);
    ImGui::Text("Render Count: %ld", Status::BatchRenderCount);
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
    array* hashtable = BehaviorRegistry::GetBehaviorInfos();

    ComponentInfo* infos = (ComponentInfo*)hashtable->vector;
    for (auto &hash : entity->GetHashes())
    {
        size_t poolID = hash % COMPONENT_TABLE_SIZE;

        ComponentInfo& info = infos[poolID];
        if (info.hash != hash)
            continue;

        char name[512] = {0};
        sprintf(name, "%s %lu", info.name, info.hash);

        ImGui::PushID(info.name);

        bool bVisible = true;

        bool bShowCollapsed = strcmp(info.name, "Transform") == 0 ? ImGui::CollapsingHeader(name) : ImGui::CollapsingHeader(name, &bVisible);
        if (bShowCollapsed)
        {
            auto it = m_guiscriptmap.find(info.name);
            if (it != m_guiscriptmap.end())
                it->second(entity);
            else
                RenderComponents(entity->getComponent(hash));
        }

        if (!bVisible)
            entity->destroy(hash);

        ImGui::PopID();
    }
}

void Editor::RenderComponents(Behavior *behavior)
{
    /*
    auto baseChunk = ComponentManager::Find(name);
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
    */
}

/*
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
*/

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

    std::cout << __FILE__ << ": " << __LINE__ << std::endl;
    
    // auto path = std::string((char*)data->Data, data->DataSize);

    // AssetsManager::LoadGameObject(path, &Editor::OutputGameObject, ref);

    ImGui::EndDragDropTarget();
}

void Editor::PopupMenuProperty() {
    ImGui::Begin("Status");

    RenderStatisticsMenu();

    ImGui::End();
}

/* ********** Propriedades ********** */
void Editor::OnPropertyTransform(Transform *transform)
{
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

        transform->setLocalRotation(Math::normalize(Math::euler(Vector3{Math::rad(euler.x), Math::rad(euler.y), Math::rad(euler.z)})));
    }
}

void Editor::OnPropertyCamera(Camera *camera)
{
    auto avail = ImGui::GetContentRegionAvail();

    bool isChanged = false;

    ImGui::Text("Rect");
    auto rect = camera->getRect();

    ImGui::Text("x");
    ImGui::SameLine();

    isChanged |= ImGui::DragFloat("##camera_rect_x", &rect.x);

    ImGui::Text("y");
    ImGui::SameLine();

    isChanged |= ImGui::DragFloat("##camera_rect_y", &rect.y);

    ImGui::Text("w");
    ImGui::SameLine();

    isChanged |= ImGui::DragFloat("##camera_rect_w", &rect.w);

    ImGui::Text("h");
    ImGui::SameLine();

    isChanged |= ImGui::DragFloat("##camera_rect_h", &rect.h);

    if (isChanged)
        camera->setRect(rect);

    ImGui::Text("Background");
    if (ImGui::ColorButton("##camera_background", (*(ImVec4*)&camera->background), ImGuiColorEditFlags_NoLabel, ImVec2(avail.x, 0.0f)))
        ImGui::OpenPopup("##camera_background_popup");

    if (ImGui::BeginPopup("##camera_background_popup"))
    {
        ImGui::ColorPicker4("##camera_background_colorpicker", camera->background.data);
        ImGui::EndPopup();
    }

    m_cameraObjectSelected = ImGui::CollapsingHeader("Render") ? camera : nullptr;

    if (m_cameraObjectSelected != nullptr)
    {
        camera->setRect(Rect{0.0f, 0.0f, 64.0f, 64.0f});

        auto rendertarget = camera->getRenderTarget();

        if (rendertarget->is_valid())
            ImGui::Image(reinterpret_cast<ImTextureID>(rendertarget->GetTextureID()), {64.0f, 64.0f}, {0.0f, 1.0f}, {1.0f, 0.0f});
    }
}

void Editor::OnPropertyMeshFilter(MeshFilter *meshFilter)
{
    if (ImGui::TreeNode("Mesh"))
    {
        for (auto &meshID : meshFilter->getMesh())
        {
            auto name = AssetsManager::GetMeshName(meshID);

            ImGui::Text("%s", name.c_str());
        }
        ImGui::TreePop();
    }
}

void Editor::OnPropertyMeshRender(MeshRender *meshRender)
{
    auto avail = ImGui::GetContentRegionAvail();
    
    if (ImGui::Button("Create", {avail.x, 0.0f}))
        popupFlags |= EditorPopupFlags_CreateMaterial;

    ImGui::Indent();
    for (MaterialID &materialID : meshRender->getMaterialIDs())
    {
        if (render_material(materialID))
        {
            meshRender->removeMaterial(materialID);
        }
    }
    ImGui::Unindent();
}

void Editor::OnPropertySkinnedMeshRender(SkinnedMeshRender *skinnedMeshRender) {
    if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
    {
        ImGui::OpenPopup("##game_object_component_menu");
    }

    auto avail = ImGui::GetContentRegionAvail();
    
    if (ImGui::Button("Create", {avail.x, 0.0f}))
        popupFlags |= EditorPopupFlags_CreateMaterial;

    ImGui::Indent();
    for (auto materialID : skinnedMeshRender->getMaterialIDs())
    {
        if (render_material(materialID))
        {
            skinnedMeshRender->removeMaterial(materialID);
        }
    }
    ImGui::Unindent();

    if (m_materialToRename)
    {
        ImGui::OpenPopup("##popup_rename_material");
    }
}

#include <gr-base/core/task/runtime/meshLoadTask.hpp>
void Editor::OnPropertyLODComponent(LODComponent* lodComponent)
{
    auto avail = ImGui::GetContentRegionAvail();

    for (auto& lod : lodComponent->meshesLOD)
    {
        ImGui::PushID(&lod);

        ImGui::Text("Distance");

        ImGui::SliderFloat("##lodComponent_levels_distance", &lod.minDistance, 0.0f, 1000.0f);

        for (auto& meshID : lod.meshes)
        {
            auto meshName = AssetsManager::GetMeshName(meshID);

            ImGui::Text("Mesh: %s", meshName.c_str());
        }

        if (ImGui::BeginDragDropTarget())
        {
            auto payload = ImGui::AcceptDragDropPayload(m_extensionDragDrop[DragDropFlags::MESH]);
            if (payload != NULL && payload->DataSize > 0)
            {
                std::vector<gtreefile *> files(payload->DataSize / sizeof(gtreefile *), nullptr);
                memcpy(files.data(), payload->Data, payload->DataSize);

                for (auto file : files)
                {
                    TaskManager::Add<MeshID, MeshLoadTask>(file->path, [ld = std::forward<LODMesh*>(&lod)](MeshID id)
                    {
                        ld->meshes.push_back(id);
                    });
                }
            }
            ImGui::EndDragDropTarget();
        }

        ImGui::PopID();
    }

    if (ImGui::Button("Add", ImVec2{avail.x, 0.0f}))
        lodComponent->meshesLOD.push_back({});
}

bool Editor::render_material(MaterialID id)
{
    auto name = AssetsManager::GetMaterialName(id);

    if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
        m_materialToRename = id;

    auto material = AssetsManager::GetMaterial(id);

    auto shader = material->GetShader(false);
    if (shader == nullptr)
    {
        dropShaderInMaterial(material);
        return false;
    }

    if (!ImGui::TreeNode(name.c_str()))
        return false;

    auto shaderParams = GetShaderParamsFromName(AssetsManager::GetShaderName(shader));

    auto shadername = gfile::getname(shaderParams.base.c_str());

    render_shader_property(shadername, material);

    render_material_property(material);

    dropShaderInMaterial(material);

    ImGui::TreePop();

    return false;
}

void Editor::render_shader_property(const std::string &name, Material *material)
{
    auto isopen = ImGui::TreeNode(name.c_str());
    if (!isopen)
        return;

    RenderShaderUniform(material, material->GetShader(false));

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
        return;

    for (auto &&[object, name, type] : property->getProperty())
    {
        switch (type)
        {
        case UniformType::MAT3:
        case UniformType::MAT4:
            continue;
        default:
            break;
        }

        ImGui::SeparatorText(name.c_str());

        switch (type)
        {
            case UniformType::VEC3:
            {
                auto value = material->getValue<Vector3>(name);
                if (value == nullptr)
                    value = material->setValue<Vector3>(name, Vector3::zero);

                if (value != nullptr)
                    ImGui::DragFloat3(("##" + std::string(name)).c_str(), (float *)value);
                break;
            }
            case UniformType::FLOAT:
            {
                auto value = material->getValue<float>(name);
                if (value == nullptr)
                    value = material->setValue<float>(name, 0.0f);

                if (value != nullptr)
                    ImGui::DragFloat(("##" + std::string(name)).c_str(), (float *)value);
                break;
            }
            /*case UniformType::int: {
            int value = 0.0f;
            if (uniform.data != nullptr)
                value = *reinterpret_cast<int *>(uniform.data);

            if (ImGui::DragInt(("##" + std::string(uniform.name)).c_str(), &value))
            {
                shader->setUniform(uniform.name, &value);
            }
            break;
        }
        case UniformType::float: {
            float value = 0.0f;
            if (uniform.data != nullptr)
                value = *reinterpret_cast<float *>(uniform.data);

            if (ImGui::DragFloat(("##" + std::string(uniform.name)).c_str(), &value))
            {
                shader->setUniform(uniform.name, &value);
            }
            break;
        }
        case UniformType::vec3: {
            Vector3 value = Vector3::zero;
            if (uniform.data != nullptr)
                value = *reinterpret_cast<Vector3 *>(uniform.data);

            if (ImGui::DragFloat3(("##" + std::string(uniform.name)).c_str(), (float *)&value))
            {
                shader->setUniform(uniform.name, &value);
            }
            break;
        }
        case UniformType::vec4: {
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

void Editor::OnPropertyLight(Light *light)
{
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
}


void Editor::OnPropertyMeshCollider(MeshCollider* meshCollider)
{
}

void Editor::OnPropertyBoxCollider(BoxCollider *boxCollider)
{
    const char* text[] {"X", "Y", "Z"};

    auto size = boxCollider->getBoxSize();
    ImGui::Text("Size");
    if (ImGui::DragFloatEx("##boxcollider_size", text, reinterpret_cast<float *>(&size), 1.0f, 0.0f, 0.0f, "%.3f"))
        boxCollider->setBoxSize(size);
}

void Editor::OnPropertyRigidbody(Rigidbody *rigidbody)
{
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
}

void Editor::OnPropertyAnimator(Animator *animator) {
    /*
    auto animation = animator->getAnimation();
    auto animName = animation != nullptr ? animation->getName() : "No Animation";

    ImGui::Text("Animation: %s", animName.c_str());

    dropAnimation(animator);

    ImGui::DragFloat("Factor", &animator->blendFactor, 0.01f, 0.0f, 1.0f);
    */
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

    std::cerr << "LoadAnimation: " << path << std::endl;
    
    // AssetsManager::LoadAnimation(path, &Animator::play, animator, true);
    
    ImGui::EndDragDropTarget();
}

void Editor::LoadReources()
{
    std::string internal_assets_path = gfile::get_data_directory() + "/assets";

    #define LOAD_ICON(path, icon_type) \
        AssetsManager::LoadTexture(internal_assets_path + path, &Editor::SetupTexture, this, icon_type)

    LOAD_ICON("/icon/icons8-file-30.png",                    IconType::ICON_FILE);
    LOAD_ICON("/icon/icons8-folder-50.png",                  IconType::ICON_FOLDER);
    LOAD_ICON("/icon/icons8-go-back-50.png",                 IconType::ICON_BACK_FILE);
    LOAD_ICON("/icon/icons8-3d-object-50.png",               IconType::ICON_GAMEOBJECT);
    LOAD_ICON("/icon/icons8-lápis-30.png",                   IconType::ICON_PENCIL);
    LOAD_ICON("/icon/icons8-lâmpada-globular-64.png",        IconType::ICON_LIGHT);
    LOAD_ICON("/icon/icons8-textura-de-renderização-32.png", IconType::ICON_TEXTURE);
    LOAD_ICON("/icon/icons8-png-24.png",                     IconType::ICON_PNG);
    LOAD_ICON("/icon/icons8-reproduzir-30.png",              IconType::ICON_PLAY_GAME);
    LOAD_ICON("/icon/icons8-parar-50.png",                   IconType::ICON_STOP_GAME);
    LOAD_ICON("/icon/icons8-resíduos-50.png",                IconType::ICON_BIN);
    LOAD_ICON("/icon/icons8-importar-50.png",                IconType::ICON_IMPORT);
    LOAD_ICON("/icon/icons8-ordenar-para-a-direita-50.png",  IconType::ICON_ASCENDENTE);
    LOAD_ICON("/icon/icons8-ordem-descendente-50.png",       IconType::ICON_DESCENDENTE);
    LOAD_ICON("/icon/icons8-editar-24.png",                  IconType::ICON_EDIT);
    LOAD_ICON("/icon/motion-graphics.png",                   IconType::ICON_ANIMATION);
    LOAD_ICON("/icon/shade.png",                             IconType::ICON_SHADER);
    LOAD_ICON("/icon/studio-set.png",                        IconType::ICON_SCENE);
    #undef LOAD_ICON
}

void Editor::SetupTexture(IconType type, grr::gTexture *texture) {
    iconsUI[type] = texture;

    if (type == IconType::ICON_PLAY_GAME) {
        s_texturePlayGame = iconsUI[IconType::ICON_PLAY_GAME];
    }
}

#include <gr-base/managers/chunkmanager.hpp>
void Editor::PostLoadGameObject(GameObjectHandle prefabID)
{
    if (m_scene == nullptr)
        return;

    GameObjectHandle gameObjectID = GameObject::Instantiate(GameObject::Get(prefabID));

    GameObject::DestroyImediate(prefabID);

    m_scene->deselect_all();
    m_scene->select(gameObjectID);
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

#include "gr-engine/plugin/pluginManager.hpp"

void Editor::PluginManager()
{
    ImVec2 winSize(200, 300);

    auto &resolution = Screen::GetResolution();
    
    ImGui::SetNextWindowPos(ImVec2{resolution.x / 2.0f, resolution.y / 2.0f} - (winSize / 2.0f), ImGuiCond_Always);
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

void Editor::PopupPluginLoad()
{
    /*
    ImVec2 winSize(200, 300);

    auto &resolution = Screen::GetResolution();
    
    ImGui::SetNextWindowPos(ImVec2{resolution.x / 2.0f, resolution.y / 2.0f} - (winSize / 2.0f), ImGuiCond_Always);
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
    */
}

void Editor::PluginEditor() {
    if (!m_plugin) { return; }

    ImVec2 winSize(200, 300);

    auto &resolution = Screen::GetResolution();
    
    ImGui::SetNextWindowPos(ImVec2{resolution.x / 2.0f, resolution.y / 2.0f} - (winSize / 2.0f), ImGuiCond_Always);
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
    m_scene = scene;

    scene->SetCamera(m_cameraID);
}

void Editor::OutputGameObject(GameObject **ref, GameObject *go)
{
    if ((*ref) != nullptr)
        GameObject::Destroy((*ref)->getCacheID());

    *ref = go;
}

void Editor::generateConfig(const std::string &file)
{
    /*
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
    */
}

void Editor::updateFileTree(gtreefile *tree_file)
{
    if (!tree_file->bdirectory)
        return;

    auto &children = tree_file->children;

    std::vector<std::filesystem::directory_entry> files;
    for (auto &dir : std::filesystem::directory_iterator(tree_file->path))
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
        child.parent        = tree_file;
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

        updateFileTree(&child);
    }
}

void Editor::select_file(gtreefile *file)
{
    auto &&[it, added] = g_tree_file_selected_map.emplace(file);
    if (added)
        g_tree_file_selected.push_back(file);
}

void Editor::unselect_file(gtreefile *file)
{
    g_tree_file_selected.erase(std::remove(g_tree_file_selected.begin(), g_tree_file_selected.end(), file), g_tree_file_selected.end());
    g_tree_file_selected_map.erase(file);
}

bool Editor::selected_file(gtreefile *file) const
{
    return g_tree_file_selected_map.find(file) != g_tree_file_selected_map.end();
}

void Editor::unselect_all_file()
{
    g_tree_file_selected.clear();
    g_tree_file_selected_map.clear();
}

void Editor::delete_file_selected()
{
    for (auto &selected : g_tree_file_selected)
        gfile::remove(selected->path);

    unselect_all_file();

    updateFileTree(&root_tree_file);
}

void Editor::drop_gmdx(std::function<void (std::string)> func)
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

    std::vector<gtreefile *> files(payload->DataSize / sizeof(gtreefile *), nullptr);
    memcpy(files.data(), payload->Data, payload->DataSize);

    for (auto &file : files)
    {
        if (file->extension == ".gmdx")
        {
            func(file->path);
            // AssetsManager::LoadGameObject(file->path, &Editor::PostLoadGameObject, this);
        }
    }

    unselect_all_file();

    ImGui::EndDragDropTarget();
}

PhysicsTask::PhysicsTask()
{}

int PhysicsTask::doInBackground()
{
    gtimer time(60.0f);

    while (Settings::bGameRunning)
    {
        time.begin_frame();

        PhysicsScene::Update(time.get_delta());

        time.end_frame();
    }

    return 0;
}


