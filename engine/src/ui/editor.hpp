#pragma once

#include <filesystem>

#include <gr-base/ecs/componentPool.hpp>
#include <gr-base/ecs/gameObject.hpp>

#include <gr-base/core/console.hpp>

#include "gr-base/script/animator.hpp"
#include "renderer.h"
#include "Global.h"

#include "guizmo/ImGuizmo.h"

#include "plugin/default.hpp"

#include <gr-render/gTexture.h>

#include <gr-base/script/default.hpp>
#include <vector>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <gr-ini/config.h>

#ifdef __cplusplus
}
#endif // __cplusplus

enum EditorPopupFlags : int64_t {
    EditorPopupFlags_EngineStatus        = 1 << 1,
    EditorPopupFlags_Settings            = 1 << 2,
    EditorPopupFlags_SkyboxEditor        = 1 << 3,
    EditorPopupFlags_Plugin              = 1 << 4,
    EditorPopupFlags_LoadPlugin          = 1 << 5,
    EditorPopupFlags_PluginEditor        = 1 << 6,
    EditorPopupFlags_AddComponent        = 1 << 7,
    EditorPopupFlags_CreateMaterial      = 1 << 8,
    EditorPopupFlags_CreateMaterialInDir = 1 << 9,
    EditorPopupFlags_FileManager         = 1 << 10,
    EditorPopupFlags_DirManager          = 1 << 11,
    EditorPopupFlags_RenameFileOrDir     = 1 << 12,
    EditorPopupFlags_CreateNewDir        = 1 << 13,
    EditorPopupFlags_CreateScript        = 1 << 14,
    EditorPopupFlags_CreateShader        = 1 << 15,
};

#define GET_FLAG_NAME(c) #c

using EditorPopupFlags_ = int;

enum IconType : int {
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
	ICON_EDIT         = 14,
	ICON_ANIMATION    = 15,
	ICON_SHADER       = 16,
	ICON_SCENE        = 17
};

typedef struct
{
    std::string name;

    void (*func)(void);
} MenuItem;

typedef struct
{
    std::string name;

    std::vector<MenuItem> items;
} MenuBar;

class Editor {
public:
    Editor();
    ~Editor();

    void RenderUI();

    void DropFile(const std::filesystem::path &path);

    void Import3DModel(const std::string &path);

    void ImportImage(const std::string &path);

    void KeyBoardUp(grm::u32 key);

    void LoadScript();

    void UnloadScript();

    void LoadReources();
    
    void SetupTexture(IconType type, grr::gTexture *texture);

    float GetFps();

    Camera *getCamera();

    std::vector<MenuBar> mMenuBarList;

private:
    static std::map<std::string, void *> m_runtimeFuncAddr;

    std::unordered_map<std::string, std::function<void(Entity *)>> mInternalScript;

    std::unordered_map<IconType, grr::gTexture *> iconsUI;

    GameObject *m_cameraObject;

    Camera *m_mainCamera;

    Plugin *m_plugin;

    ImGuiViewport *m_viewport;

    grr::gTexture *s_texturePlayGame;

    GameObject *s_objectSelected;

    EditorPopupFlags_ popupFlags;

    ImGuiIO *mImGuiIO;

    grm::u8 m_currentPage;

    bool bFileDoubleClick;

    bool bCutAllFile;

    bool bCopyAllFile;

    bool bViewPortFocused;

    bool bOpenPopupInfoDir;

    bool bOpenPopupOptionGameObject;

    bool bBuildDock;

    ImVec4 FPSColor;

    ImVec2 m_lastViewportSize;

    ImGuizmo::OPERATION m_guizmoOperation;

    ImGuizmo::MODE m_guizmoManipulate;

    std::unordered_map<std::filesystem::path, bool> mDirSelectedMap;

    std::vector<std::filesystem::path> mDirSelectedList;

    std::filesystem::path m_currentPath;

    std::filesystem::path m_dirSelected;

    std::shared_ptr<Config> m_config;

    Material *m_materialToRename;

    gConfig *m_settings;

    struct LastTransform {
        Vector3 position;
        Quaternion rotation;
        Vector3 scale;
    } mLastTransform;

    void RenderDocking();

    void BuildDock(ImGuiID dockspace_id, ImGuiDockNodeFlags dockspace_flags);

    void PopupComponentMenu();

    void RenderObjectUI();

    void dropGameObjectInHierarchy(Scene *scene);

    void RenderSceneObject(Scene *scene);

    void RenderObjectPage(std::vector<GameObject *> list, int page);

    void ReadHierarchy(Transform* transform);

    void dragDropGameObject(Transform *transform, const std::string &name);

    void PopupMenuAdd();

    void PopupOptionsGameObject();

    void PopupRenameGameObject();

    void RenderViewportUI();

    void DropSceneInViewport();

    void RenderScene();

    void UpdateResizeSceneFramebuffer();

    void PlayGame();

    void StopGame();

    void RenderGuizmo();

    void DropGameObjectInScene();

    void PopupMenuCreateScene();

    void OpenFileDoubleClick(std::filesystem::path path);

    void PopupFileAndDirmanager();

    void PopupFileManager();

    void PopupDirManager();

    void PopupRenameFileOrDir();

    void PopupCreateDirectory();

    void PopupCreateScript();

    void PopupCreateShader();

    void RenderOtherUI();

    void RenderTreeNodeFile(std::filesystem::path path);

    void RenderFiles();

    void dropTargetGameObjectInFile(std::string path);

    void RenderConsoleWindow();

    void RenderConsole();

    void RenderLog(Log &info);

    void RenderSettings();

    void ConfigureVSCode();

    void RenderPropertiUI();

    void RenderStatisticsMenu();

    void RenderObjectInfo();

    void dropScriptInAddComponent();

    void RenderScriptFromEntity(Entity *entity);

    bool RenderComponents(const std::string &name, Component *component);

    void RenderField(Field &field, Component *component);

    void renderDropGameObject(GameObject **ref);

    void PopupAddComponent();

    void PopupMenuProperty();

    bool ProgressLoad(int max, int current);

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

    void TextureInfo(Texture*& texture);

    // Loader    
    void PostLoadGameObject(GameObject *gameObject);

    void CreateScriptTemplate(const std::string &path, const std::string &name);

    void CreateShaderTemplate(const std::string &path, const std::string &name);

    // PopupMenu
    void PopupMenuSettings();

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

    // Scene
    void SaveScene(const std::string &name, Scene *scene);

    // UI
    void showShaderListPopup();

    void renderShaderList();

    void showMeshListPopup();

    void renderMeshList();

    // 
    void SkyboxEditor();

    void PluginManager();

    void PopupPluginLoad();

    void PluginEditor();

    void ScenePostEcecute(Scene *scene);

    template <typename T, typename... Args>
    bool RenderProperty(Args&&... args);

    static void OutputGameObject(GameObject **ref, GameObject *go);

    void refreshDockMenuBar(const std::string &file);

    static gConfigStep_ foreach_ini(gConfigType_ type, void *data, void *editor);

    static gConfigStep_ foreach_settings(gConfigType_ type, void *data, void *instance);

    void generateConfig(const std::string &file);
};

template <typename T, typename... Args>
inline bool Editor::RenderProperty(Args&&... args) {
    if (!s_objectSelected->hasComponent<T>()) return false;

    return std::invoke(std::bind(std::forward<Args>(args)..., s_objectSelected->getComponent<T>()));
}

