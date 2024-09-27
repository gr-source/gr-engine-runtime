#pragma once

#include <filesystem>

#include "core/ecs/componentPool.hpp"

#include "Global.h"
#include "renderer.h"

#include "guizmo/ImGuizmo.h"

#include <gr-render/gTexture.h>

class Editor {
public:
    static void Initialize();

    static void Shutdown();

    static void RenderUI();

    static void DropFile(const std::filesystem::path &path);

    static void KeyBoardUp(grm::u32 key);

    static void LoadScript();

    static void UnloadScript();

    static void LoadReources();

    static float GetFps();

private:
    static ImGuiViewport *m_viewport;

    static grr::gTexture *s_texturePlayGame;

    static std::unordered_map<std::string, std::shared_ptr<Scene>> m_scenes;

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

    static void DropGameObjectInScene();

    static void PopupMenuCreateScene();

    static void RenderFilesUI();

    static void RenderTreeNodeFile(std::filesystem::path path);

    static void OpenFileDoubleClick(std::filesystem::path path);

    static void PopupDirmanager();

    static void PopupFileManager();

    static void PopupCreateScript();

    static void PopupCreateDirectory();

    static void RenderConsoleUI();

    static void RenderSettings();

    static void ConfigureVSCode();

    static void RenderPropertiUI();

    static void RenderStatisticsMenu();

    static void RenderGameObjectPropertiUI();

    static void RenderField(Field &field, Component *component);

    static void PopupAddComponent();

    static bool ProgressLoad(int max, int current);

    enum NavegationFlags : grm::u8 {
        NavegationFlags_default = 1 << 1,
        NavegationFlags_console = 1 << 2,
        NavegationFlags_files   = 1 << 3,
        NavegationFlags_models  = 1 << 4,
    };

    static void OnPropertyTransform(Transform* transform);
    
    static void OnPropertyMeshFilter(MeshFilter* MeshFilter);
    
    static void OnPropertyMeshRender(MeshRender* meshRender);

    static void OnPropertyAnimator(Animator* animator);

    static void OnPropertyLight(Light *light);

    static void TextureInfo(Texture*& texture);
    
    static void PostLoadGameObject(GameObject *gameObject);

    static void CreateScriptTemplate(const std::string &path, const std::string &name);

    // PopupMenu
    static void PopupMenuSettings();

    // Script
    static void BuildScript();

    static void ReconfigScript();

    static void BuildProject(std::string path);

    // Scene
    static void SaveScene(const std::string &name, Scene *scene);
};
