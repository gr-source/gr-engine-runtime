#pragma once

#include <filesystem>

#include "ecs/componentPool.hpp"

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


class Editor {
public:
    static void Initialize();

    static void Shutdown();

    static void RenderUI();

    static void DropFile(const std::filesystem::path &path);

    static void Import3DModel(const std::string &path);

    static void ImportImage(const std::string &path);

    static void KeyBoardUp(grm::u32 key);

    static void LoadScript();

    static void UnloadScript();

    static void LoadReources();
    
    static void SetupTexture(IconType type, grr::gTexture *texture);

    static float GetFps();

private:
    static ImGuiViewport *m_viewport;

    static grr::gTexture *s_texturePlayGame;

    static GameObject *s_objectSelected;

    static ImGuiIO* s_io;

    static grm::u8 m_currentPage;

    static bool bFileDoubleClick;

    static bool bCutAllFile;

    static bool bCopyAllFile;

    static bool bViewPortFocused;

    static bool bOpenPopupInfoDir;

    static bool bScriptReloaded;

    static bool bOpenPopupOptionGameObject;

    static bool pPopupFileManager;

    static bool bBuildDock;

    static ImVec4 FPSColor;

    static ImVec2 m_lastViewportSize;

    static ImGuizmo::OPERATION m_guizmoOperation;

    static ImGuizmo::MODE m_guizmoManipulate;

    static grm::u32 s_maxProgress;

    static std::unordered_map<std::filesystem::path, bool> fLisDirSelected;

    static std::filesystem::path m_currentPath;

    static std::filesystem::path m_SelectedPath;

    static std::shared_ptr<Config> m_config;

    static void BuildDock(ImGuiID dockspace_id, ImGuiDockNodeFlags dockspace_flags);

    static void PopupComponentMenu();

    static void RenderDocking();

    static void RenderScenesUI();

    static void RenderHierarchi();

    static void RenderPage(std::vector<GameObject *> &list, int page);

    static void ReadHierarchy(Transform* transform);

    static void PopupMenuAdd();

    static void PopupOptionsGameObject();

    static void PopupRenameGameObject();

    static void RenderViewportUI();

    static void DropSceneInViewport();

    static void RenderScene();

    static void PlayGame();

    static void StopGame();

    static void RenderGuizmo();

    static void DropGameObjectInScene();

    static void PopupMenuCreateScene();

    static void RenderFilesUI();

    static void RenderTreeNodeFile(std::filesystem::path path);

    static void DropTargetGameObjectInFile(const std::string &path);

    static void OpenFileDoubleClick(std::filesystem::path path);

    static void PopupDirmanager();

    static void PopupFileManager();

    static void PopupCreateScript();

    static void PopupCreateDirectory();

    static void RenderOtherUI();

    static void RenderConsoleWindow();

    static void RenderConsole();

    static void RenderLog(Log &info);

    static void RenderSettings();

    static void ConfigureVSCode();

    static void RenderPropertiUI();

    static void RenderStatisticsMenu();

    static void RenderGameObjectPropertiUI();

    static bool RenderComponents(const std::string &name, Component *component);

    static void RenderField(Field &field, Component *component);

    static void PopupAddComponent();

    static bool ProgressLoad(int max, int current);

    enum NavegationFlags : grm::u8 {
        NavegationFlags_default = 1 << 1,
        NavegationFlags_console = 1 << 2,
        NavegationFlags_files   = 1 << 3,
        NavegationFlags_models  = 1 << 4,
    };

    static bool OnPropertyTransform(Transform *transform);
    
    static bool OnPropertyMeshFilter(MeshFilter *meshFilter);

    static void RenderUVConfig(MeshFilter *meshFilter);
    
    static bool OnPropertyMeshRender(MeshRender *meshRender);

    static void InsertTexture(Material *material);

    static bool OnPropertyLight(Light *light);

    static bool OnPropertyBoxCollider(BoxCollider *boxCollider);

    static bool OnPropertyRigidbody(Rigidbody *rigidbody);

    static void OnPropertyAnimator(Animator* animator);

    static void TextureInfo(Texture*& texture);

    // Loader    
    static void PostLoadGameObject(GameObject *gameObject);

    static void SetupTextureMaterial(Material *material, int id, grr::gTexture *texture);

    static void CreateScriptTemplate(const std::string &path, const std::string &name);

    // PopupMenu
    static void PopupMenuSettings();

    static void PopupRenameFile();

    // Project
    static void BuildProject();

    static void PostBuildScriptFromProject();

    // Script
    static void ReconfigScript();

    static void BuildScript();

    static void CompileScript(std::string path, std::function<void()> func);

    // Scene
    static void SaveScene(const std::string &name, Scene *scene);
};
