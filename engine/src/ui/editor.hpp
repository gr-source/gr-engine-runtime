#pragma once

#include <filesystem>

#include <gr-base/ecs/componentPool.hpp>
#include <gr-base/ecs/gameObject.hpp>

#include <gr-base/core/console.hpp>

#include "renderer.h"
#include "Global.h"

#include "guizmo/ImGuizmo.h"

#include "plugin/default.hpp"

#include <gr-render/gTexture.h>

enum EditorPopupFlags : int {
    EditorPopupFlags_EngineStatus   = 1 << 1,
    EditorPopupFlags_Settings       = 1 << 2,
    EditorPopupFlags_SkyboxEditor   = 1 << 2,
    EditorPopupFlags_Plugin         = 1 << 3,
    EditorPopupFlags_LoadPlugin     = 1 << 4,
    EditorPopupFlags_PluginEditor   = 1 << 5,
    EditorPopupFlags_AddComponent   = 1 << 6,
    EditorPopupFlags_CreateMaterial = 1 << 7,
};

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

private:
    std::unordered_map<IconType, grr::gTexture *> iconsUI;
    
    GameObject *m_cameraObject;

    GameObject *tempCam;

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

    bool bScriptReloaded;

    bool bOpenPopupOptionGameObject;

    bool pPopupFileManager;

    bool bBuildDock;

    ImVec4 FPSColor;

    ImVec2 m_lastViewportSize;

    ImGuizmo::OPERATION m_guizmoOperation;

    ImGuizmo::MODE m_guizmoManipulate;

    grm::u32 s_maxProgress;

    std::unordered_map<std::filesystem::path, bool> fLisDirSelected;

    std::filesystem::path m_currentPath;

    std::filesystem::path m_SelectedPath;

    std::shared_ptr<Config> m_config;

    Material *m_materialToRename;

    struct LastTransform {
        Vector3 position;
        Quaternion rotation;
        Vector3 scale;
    } mLastTransform;

    void RenderDocking();

    void BuildDock(ImGuiID dockspace_id, ImGuiDockNodeFlags dockspace_flags);

    void PopupComponentMenu();

    void RenderScenesUI();

    void RenderHierarchi();

    void RenderPage(std::vector<GameObject *> &list, int page);

    void ReadHierarchy(Transform* transform);

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

    void RenderFilesUI();

    void RenderTreeNodeFile(std::filesystem::path path);

    void DropTargetGameObjectInFile(std::string path);

    void OpenFileDoubleClick(std::filesystem::path path);

    void PopupDirmanager();

    void PopupFileManager();

    void PopupCreateScript();

    void PopupCreateDirectory();

    void RenderOtherUI();

    void RenderConsoleWindow();

    void RenderConsole();

    void RenderLog(Log &info);

    void RenderSettings();

    void ConfigureVSCode();

    void RenderPropertiUI();

    void RenderStatisticsMenu();

    void RenderGameObjectPropertiUI();

    bool RenderComponents(const std::string &name, Component *component);

    void RenderField(Field &field, Component *component);

    void PopupAddComponent();

    void PopupMenuProperty();

    bool ProgressLoad(int max, int current);

    enum NavegationFlags : grm::u8 {
        NavegationFlags_default = 1 << 1,
        NavegationFlags_console = 1 << 2,
        NavegationFlags_files   = 1 << 3,
        NavegationFlags_models  = 1 << 4,
    };

    bool OnPropertyTransform(Transform *transform);

    bool OnPropertyCamera(Camera *camera);
    
    bool OnPropertyMeshFilter(MeshFilter *meshFilter);

    void RenderUVConfig(MeshFilter *meshFilter);
    
    bool OnPropertyMeshRender(MeshRender *meshRender);

    bool RenderMaterial(Material *material);

    void InsertTexture(Material *material);

    bool OnPropertyLight(Light *light);

    bool OnPropertyBoxCollider(BoxCollider *boxCollider);

    bool OnPropertyRigidbody(Rigidbody *rigidbody);

    bool OnPropertyAnimator(Animator* animator);

    void TextureInfo(Texture*& texture);

    // Loader    
    void PostLoadGameObject(GameObject *gameObject);

    void CreateScriptTemplate(const std::string &path, const std::string &name);

    // PopupMenu
    void PopupMenuSettings();

    void RenameMaterial();

    void CreateMaterial();

    void PopupCreateMaterial();

    void PopupRenameFile();

    // Project
    void BuildProject();

    void PostBuildScriptFromProject();

    // Script
    void ReconfigScript();

    void BuildScript();

    void CompileScript(std::string path, std::function<void()> func);

    // Scene
    void SaveScene(const std::string &name, Scene *scene);

    void setActiveCamera(bool value);

    // UI
    void showShaderListPopup();

    void renderShaderList();

    void showMeshListPopup();

    void renderMeshList();

    static GameObject *CreatePrimitive(int type);

    // 
    void SkyboxEditor();

    void PluginManager();

    void PluginLoad();

    void PluginEditor();

    void ScenePostEcecute(Scene *scene);

    template <typename T, typename... Args>
    bool RenderProperty(Args&&... args);
};

template <typename T, typename... Args>
inline bool Editor::RenderProperty(Args&&... args) {
    if (!s_objectSelected->hasComponent<T>()) return false;

    return std::invoke(std::bind(std::forward<Args>(args)..., s_objectSelected->getComponent<T>()));
}

