#pragma once

#include "Global.h"
#include "renderer.h"

#include "filmbox/Filmbox.h"

#include "guizmo/ImGuizmo.h"

class Game {
public:
    Game();
    
    void Update();

    #if !IS_GAME
    void Gui();

    void RenderDocking();

    void RenderScenesUI();

    void ReadHierarchy(std::vector<GameObject *>& list, GameObject* gameObject);

    void RenderViewportUI();

    void RenderFilesUI();

    void RenderConsoleUI();

    void RenderPropertiUI();

    void RenderRectSlide(const Color& color);

    void loadReources();

    void ProgressLoad();

    enum NavegationFlags : u8 {
        NavegationFlags_default = 1 << 1,
        NavegationFlags_console = 1 << 2,
        NavegationFlags_files   = 1 << 3,
        NavegationFlags_models  = 1 << 4,
    };

    static void ImportFiles(const std::vector<std::filesystem::path>& files);

    #endif

private:
    /* Seleciona um objeto no scenes(UI)*/
    GameObject* objectselected;
    
    /* Textura do play button */
    Sprite* texturePlayGame;

    std::mutex m_mutex;

    #if !IS_GAME
    ImGuiIO* m_io;

    ImGuizmo::OPERATION m_guizmoOperation;

    ImGuizmo::MODE m_guizmoManipulate;

    void OnPropertyTransform(Transform* transform);
    
    void OnPropertyMeshRender(MeshRender* meshRender);

    void OnPropertyCameraComponent(CameraComponent* cameraComponent);

    void OnPropertyLightComponent(LightComponent* lightComponent);

    void TextureInfo(Texture2D*& texture);

    void MenuAdd();

    #endif
};
