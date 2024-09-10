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

private:
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

    static ImVec4 FPSColor;

    static ImVec2 m_lastViewportSize;

    static ImGuizmo::OPERATION m_guizmoOperation;

    static ImGuizmo::MODE m_guizmoManipulate;

    static grm::u32 s_maxProgress;

    static std::unordered_map<std::filesystem::path, bool> fLisDirSelected;

    static std::filesystem::path m_currentPath;

    static void RenderDocking();

    static void CloseProject();

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

    static void PopupDirmanager();

    static void PopupFileManager();

    static void PopupCreateDirectory();

    static void RenderConsoleUI();

    static void RenderPropertiUI();

    static void RenderStatisticsMenu();

    static void RenderGameObjectPropertiUI();

    static void RenderField(Field &field, Component *component);

    static void PopupAddComponent();

    static void ProgressLoad();

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
};
