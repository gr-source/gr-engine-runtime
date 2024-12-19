#pragma once

#include <filesystem>

#include <ecs/componentPool.hpp>

#include "core/console.hpp"

#include "renderer.h"
#include "Global.h"

#include "guizmo/ImGuizmo.h"

#include <gr-render/gTexture.h>


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

enum UIPopup : grm::u32 {
    UIPopup_PopupAddComponent = 1 << 1, 
};

using UIPopup_ = grm::u32;


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

    ImGuiViewport *m_viewport;

    grr::gTexture *s_texturePlayGame;

    GameObject *s_objectSelected;

    ImGuiIO* s_io;

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

    UIPopup_ uIPopup;

    GameObject *createEditorCamera();

    void BuildDock(ImGuiID dockspace_id, ImGuiDockNodeFlags dockspace_flags);

    void PopupComponentMenu();

    void RenderDocking();

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

    void PlayGame();

    void StopGame();

    void loadCamera(GameObject *camera);

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

    void rTabObjectInfo(bool &isOpenPopup);

    bool RenderComponents(const std::string &name, Component *component);

    void RenderField(Field &field, Component *component);

    void PopupAddComponent();

    bool ProgressLoad(int max, int current);

    enum NavegationFlags : grm::u8 {
        NavegationFlags_default = 1 << 1,
        NavegationFlags_console = 1 << 2,
        NavegationFlags_files   = 1 << 3,
        NavegationFlags_models  = 1 << 4,
    };

    bool OnPropertyTransform(Transform *transform);
    
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

    void SetupTextureMaterial(Material *material, int id, grr::gTexture *texture);

    void CreateScriptTemplate(const std::string &path, const std::string &name);

    // PopupMenu
    void PopupMenuSettings();

    void PopupRenameMaterial();

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
};
