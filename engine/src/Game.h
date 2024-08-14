#pragma once

#include <filesystem>

#include "Global.h"
#include "renderer.h"

#include "guizmo/ImGuizmo.h"

#include <gr-render/gTexture.h>

class Game {
public:
    static void Initialize();

    static void RenderUI();

    static void DropFile(const std::filesystem::path &path);

    static void KeyBoardUp(grm::u32 key);

private:
    /* Seleciona um objeto no scenes(UI)*/
    static GameObject *s_objectSelected;
    
    /* Textura do play button */
    static grr::gTexture *s_texturePlayGame;

    static ImGuiIO* s_io;

    static bool bFileDoubleClick;

    static bool bCutAllFile;

    static bool bCopyAllFile;

    static bool bViewPortFocused;

    static bool bOpenPopupInfoDir;

    static bool bScriptReloaded;

    static bool bMenuAdd;

    static ImVec4 FPSColor;

    static ImGuizmo::OPERATION m_guizmoOperation;

    static ImGuizmo::MODE m_guizmoManipulate;

    static u32 s_maxProgress;

    static std::unordered_map<std::filesystem::path, bool> fLisDirSelected;

    static std::filesystem::path m_currentFile;

    static void RenderDocking();

    static void RenderScenesUI();

    static void ReadHierarchy(Transform* transform);

    static void RenderViewportUI();

    static void RenderFilesUI();

    static void PopupDirmanager();

    static void PopupFileManager();

    static void PopupCreateDirectory();

    static void RenderConsoleUI();

    static void RenderPropertiUI();

    static void loadReources();

    static void ProgressLoad();

    enum NavegationFlags : u8 {
        NavegationFlags_default = 1 << 1,
        NavegationFlags_console = 1 << 2,
        NavegationFlags_files   = 1 << 3,
        NavegationFlags_models  = 1 << 4,
    };

    static void PlayGame();

    static void StopGame();

    static void OnPropertyTransform(Transform* transform);
    
    static void OnPropertyAnimator(Animator* animator);

    static void OnPropertyMeshRender(MeshRender* meshRender);

    // static void OnPropertyCameraComponent(CameraComponent* cameraComponent);

    static void OnPropertyLightComponent(Light *light);

    // static void OnPropertyChunkController(ChunkController* chunkController);

    static void TextureInfo(Texture*& texture);

    static void MenuAdd();
};
