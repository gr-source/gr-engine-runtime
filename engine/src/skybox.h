#pragma once

#include "framebuffer/skyboxbuffer.h"

#include "resources/Shader.h"

#include "bufferManager.h"

#include "common.h"

class Skybox {
public:
    Skybox();

    static Skybox& getInstance();

    bool play;
    
    void update();
    void render();

    Cubemap* getIrradiance();
private:
    std::unique_ptr<BufferManager> bufferManager;
    Shader* m_backgroundShader;
    Shader* m_skyboxShader;
    float distance;

    Texture* m_texture;

    GLuint count;

    float currentTime;

    Matrix4x4 captureProjection;
    std::array<Matrix4x4, 6> captureViews;

    std::unique_ptr<Skyboxbuffer> m_skyboxbuffer;

    void prepareShader();

    void setupSkybox(Shader* shader);

    void GenerateMap(std::function<void(u32)> func, Texture* texture);
};
