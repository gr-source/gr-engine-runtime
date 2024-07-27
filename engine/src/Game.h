#pragma once

#include "Global.h"
#include "renderer.h"

#include "filmbox/Filmbox.h"

#include "guizmo/ImGuizmo.h"

class Game {
public:
    Game();

    #if !IS_GAME
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

    #endif

private:
    /* Seleciona um objeto no scenes(UI)*/
    GameObject* objectselected;
    
    /* Textura do play button */
    Texture* texturePlayGame;

    std::mutex m_mutex;

    #if !IS_GAME
    ImGuiIO* m_io;

    bool menuAdd;

    ImVec4 FPSColor;

    ImGuizmo::OPERATION m_guizmoOperation;

    ImGuizmo::MODE m_guizmoManipulate;

    void OnPropertyTransform(Transform* transform);
    
    void OnPropertyAnimator(Animator* animator);

    void OnPropertyMeshRender(MeshRender* meshRender);

    void OnPropertyCameraComponent(CameraComponent* cameraComponent);

    void OnPropertyLightComponent(LightComponent* lightComponent);

    void OnPropertyChunkController(ChunkController* chunkController);

    void TextureInfo(Texture*& texture);

    void MenuAdd();

    #endif
};
