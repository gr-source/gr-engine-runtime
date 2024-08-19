#pragma once

#include <gr-render/gRender.h>

#include <functional>

class PostProcessing;

class Skybox {
public:
    static void Initialize();

    static void Shutdown();

    static void Renderer();

private:
    static grr::gVertexArray *m_vertexArray;

    static grr::gShader *m_irradianceShader;

    static grr::gShader *m_skyShader;

    static grr::gShader *m_shader;

    static PostProcessing *m_postProcess;

    static grm::u32 m_count;

    static grm::u32 m_vbo;
    
    static grm::u32 m_ebo;

    static void CreateCube();
    
    static bool SetupShader(grr::gShader *shader, const std::string &path);

    static void GenerateMap(std::function<void(grm::u32)> func, grr::gTexture *texture);
};



/*
#include "framebuffer/skyboxbuffer.h"

#include "common.h"

#include <gr-render/gRender.h>

class Skybox {
public:
    Skybox();

    static Skybox& getInstance();

    bool play;
    
    void update();
    void render();

    Cubemap* getIrradiance();
private:
    grr::gVertexArray *m_vertexArray;

    grm::u32 m_ebo;

    grm::u32 m_vbo;

    grr::gShader *m_shader;

    // Shader* m_backgroundShader;
    
    // Shader* m_skyboxShader;

    float distance;

    Texture* m_texture;

    GLuint count;

    float currentTime;

    Matrix4x4 captureProjection;
    std::array<Matrix4x4, 6> captureViews;

    std::unique_ptr<Skyboxbuffer> m_skyboxbuffer;

    void loadShader();

    void createVertexArray();

    void SetupBackgroundTexture(Texture* texture);

    void setupSkybox(Shader* shader);

    void setupIrradiance(Shader* shader);

    void GenerateMap(std::function<void(u32)> func, Texture* texture);
};
*/
