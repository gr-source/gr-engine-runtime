#pragma once

#include <filesystem>

#include "Global.h"
#include "renderer.h"

#include "guizmo/ImGuizmo.h"

class Game {
public:
    Game();

    void dropFile(const std::filesystem::path &path);

    void Gui();

    void RenderDocking();

    void RenderScenesUI();

    void ReadHierarchy(Transform* transform);

    void RenderViewportUI();

    void RenderFilesUI();

    void RenderConsoleUI();

    void RenderPropertiUI();

    void loadReources();

    void ProgressLoad();

    enum NavegationFlags : u8 {
        NavegationFlags_default = 1 << 1,
        NavegationFlags_console = 1 << 2,
        NavegationFlags_files   = 1 << 3,
        NavegationFlags_models  = 1 << 4,
    };

private:
    /* Seleciona um objeto no scenes(UI)*/
    GameObject* objectselected;
    
    /* Textura do play button */
    Texture* texturePlayGame;

    ImGuiIO* m_io;

    bool bScriptReloaded;
    bool bMenuAdd;

    ImVec4 FPSColor;

    ImGuizmo::OPERATION m_guizmoOperation;

    ImGuizmo::MODE m_guizmoManipulate;

    void OnPropertyTransform(Transform* transform);
    
    void OnPropertyAnimator(Animator* animator);

    void OnPropertyMeshRender(MeshRender* meshRender);

    // void OnPropertyCameraComponent(CameraComponent* cameraComponent);

    void OnPropertyLightComponent(Light *light);

    // void OnPropertyChunkController(ChunkController* chunkController);

    void TextureInfo(Texture*& texture);

    void MenuAdd();
};
