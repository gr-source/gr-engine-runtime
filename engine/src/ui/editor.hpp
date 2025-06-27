#pragma once

#include <cstdint>
#include <filesystem>

#include <gr-base/ecs/componentPool.hpp>
#include <gr-base/ecs/gameObject.hpp>

#include <gr-base/core/console.hpp>

#include "gr-base/core/scene.hpp"
#include "gr-base/script/animator.hpp"
#include "renderer.h"
#include "Global.h"

#include "guizmo/ImGuizmo.h"

#include "plugin/default.hpp"

#include <gr-render/gTexture.h>

#include <gr-base/script/default.hpp>
#include <string>
#include <vector>

#include "viewlayout.hpp"

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

#include <gr-ini/config.h>

#ifdef __cplusplus
}
#endif // __cplusplus

using EditorPopupFlags_ = std::uint32_t;

enum EditorPopupFlags : EditorPopupFlags_
{
    EditorPopupFlags_EngineStatus           = 1 << 1,
    EditorPopupFlags_Settings               = 1 << 2,
    EditorPopupFlags_SkyboxEditor           = 1 << 3,
    EditorPopupFlags_Plugin                 = 1 << 4,
    EditorPopupFlags_LoadPlugin             = 1 << 5,
    EditorPopupFlags_PluginEditor           = 1 << 6,
    EditorPopupFlags_AddComponent           = 1 << 7,
    EditorPopupFlags_CreateMaterial         = 1 << 8,
    EditorPopupFlags_CreateMaterialInDir    = 1 << 9,
    EditorPopupFlags_FileManager            = 1 << 10,
    EditorPopupFlags_DirManager             = 1 << 11,
    EditorPopupFlags_RenameFileOrDir        = 1 << 12,
    EditorPopupFlags_CreateNewDir           = 1 << 13,
    EditorPopupFlags_CreateScript           = 1 << 14,
    EditorPopupFlags_CreateShader           = 1 << 15,
    EditorPopupFlags_RenameGameObject       = 1 << 16,
    EditorPopupFlags_SceneAddOption         = 1 << 17,
    EditorPopupFlags_CreateScene            = 1 << 18,
};

#define GET_FLAG_NAME(c) #c


enum IconType : int {
    ICON_FOLDER         = 0,
    ICON_FILE           = 1,
    ICON_BACK_FILE      = 2,
    ICON_GAMEOBJECT     = 3,
	ICON_PENCIL         = 4,
	ICON_LIGHT          = 5,
	ICON_TEXTURE        = 6,
	ICON_PNG            = 7,
	ICON_PLAY_GAME      = 8,
	ICON_STOP_GAME      = 9,
	ICON_BIN            = 10,
	ICON_IMPORT         = 11,
	ICON_ASCENDENTE     = 12,
	ICON_DESCENDENTE    = 13,
	ICON_EDIT           = 14,
	ICON_ANIMATION      = 15,
	ICON_SHADER         = 16,
	ICON_SCENE          = 17,
	ICON_SCRIPT         = 18
};

typedef struct gtreefile
{
    std::string path;

    std::string name;

    std::string extension;

    grr::gTexture **icon;

    bool bdirectory;

    std::vector<gtreefile> children;
} gtreefile;

class Editor : public ViewLayout
{
public:
    Editor();
    ~Editor() override;

    void onRender() override;

    void onFocus() override;

    void onBlur() override;

    void drop(void *data) override;

    void LoadScript();

    void UnloadScript();

    void LoadReources();
    
    void SetupTexture(IconType type, grr::gTexture *texture);

    Camera *getCamera();

private:
    static std::map<std::string, void *> m_runtimeFuncAddr;

    std::unordered_map<std::string, std::function<void(Entity *)>> mInternalScript;

    std::unordered_map<IconType, grr::gTexture *> iconsUI;

    std::vector<gtreefile *> g_tree_file_selected;

    Scene *m_scene;

    GameObject *m_cameraObject;

    Camera *m_mainCamera;

    Plugin *m_plugin;

    ImGuiViewport *m_viewport;

    grr::gTexture *s_texturePlayGame;

    EditorPopupFlags_ popupFlags;

    ImGuiIO *mImGuiIO;

    grm::u8 m_currentPage;

    bool bBuildDock;

    ImVec4 FPSColor;

    ImVec2 m_lastViewportSize;

    gtreefile root_tree_file;

    gtreefile *g_tree_file;

    ImGuizmo::OPERATION m_guizmoOperation;

    ImGuizmo::MODE m_guizmoManipulate;

    std::vector<std::filesystem::path> mDirSelectedList;

    std::filesystem::path m_currentPath;

    std::filesystem::path m_dirSelected;

    Material *m_materialToRename;

    gConfig *m_settings;

    void create_dock();

    void BuildDock(ImGuiID dockspace_id, ImGuiDockNodeFlags dockspace_flags);

    void PopupComponentMenu();

    void create_window_hierarchy();

    void create_render_hierarchy();

    void dropGameObjectInHierarchy(Scene *scene);

    void RenderSceneObject(Scene *scene);

    void ReadHierarchy(Scene *scene, Transform* transform);

    void dragDropGameObject(Transform *transform, const std::string &name);

    void PopupMenuAdd();

    void create_window_viewport();

    void create_render_viewport();

    void drop_scene_file_in_viewport();

    void render_scene(Scene *scene);

    void UpdateResizeSceneFramebuffer();

    void create_render_guizmo(Scene *scene);

    void PlayGame();

    void StopGame();

    void DropGameObjectInScene();

    void OpenFileDoubleClick(std::filesystem::path path);

    void PopupFileAndDirmanager();

    void PopupFileManager();

    void PopupDirManager();

    void PopupRenameFileOrDir();

    void PopupCreateDirectory();

    void PopupCreateScript();

    void PopupCreateShader();

    void create_window_browser();

    void create_render_browser();

    void RenderTreeNodeFile(gtreefile &tree_file);

    void RenderFiles();

    void dropTargetGameObjectInFile(std::string path);

    void create_window_console();

    void create_render_console();

    void RenderLog(Log &info);

    void ConfigureVSCode();

    void create_window_property();

    void create_render_property();

    void create_call_popup();

    void create_popup();

    void create_popup_create_scene();

    void create_popup_add_component();

    void create_popup_rename_object();

    void RenderStatisticsMenu();

    void dropScriptInAddComponent();

    void RenderScriptFromEntity(Entity *entity);

    bool RenderComponents(const std::string &name, Component *component);

    void RenderField(Field &field, Component *component);

    void renderDropGameObject(GameObject **ref);

    void PopupMenuProperty();

    bool OnPropertyTransform(Transform *transform);

    bool OnPropertyCamera(Camera *camera);
    
    bool OnPropertyMeshFilter(MeshFilter *meshFilter);

    bool OnPropertyMeshRender(MeshRender *meshRender);

    bool OnPropertySkinnedMeshRender(SkinnedMeshRender *skinnedMeshRender);

    // bool OnPropertyMeshCollider(MeshCollider *meshCollider);

    bool RenderMaterial(Material *material);

    void RenderShaderUniform(Material *material, grr::gShader *shader);

    void dropShaderInMaterial(Material *material);

    void InsertTexture(Material *material);

    bool OnPropertyLight(Light *light);

    bool OnPropertyBoxCollider(BoxCollider *boxCollider);

    bool OnPropertyRigidbody(Rigidbody *rigidbody);

    bool OnPropertyAnimator(Animator* animator);

    void dropAnimation(Animator *animator);

    // Loader    
    void PostLoadGameObject(GameObject *gameObject);

    void CreateScriptTemplate(const std::string &path, const std::string &name);

    void CreateShaderTemplate(const std::string &path, const std::string &name);

    // PopupMenu
    void RenameMaterial();

    void CreateMaterial();

    void PopupCreateMaterial();


    // Project
    void BuildProject();

    void PostBuildScriptFromProject();

    // Script
    void ReconfigScript();

    void BuildScript();

    void CompileScript(std::string path, std::function<void()> func);

    void SkyboxEditor();

    void PluginManager();

    void PopupPluginLoad();

    void PluginEditor();

    void set_scene(Scene *scene);

    template <typename T, typename... Args>
    bool RenderProperty(Args&&... args);

    static void OutputGameObject(GameObject **ref, GameObject *go);

    static gConfigStep_ foreach_settings(gConfigType_ type, void *data, void *instance);

    void generateConfig(const std::string &file);

    void updateFileTree(gtreefile &tree_file);
};

template <typename T, typename... Args>
inline bool Editor::RenderProperty(Args&&... args)
{
    if (m_scene == nullptr || !m_scene->has_object_selected())
    {
        return false;
    }

    auto go = m_scene->get_object_selected().back();
    if (!go->hasComponent<T>()) return false;

    return std::invoke(std::bind(std::forward<Args>(args)..., go->getComponent<T>()));
}

