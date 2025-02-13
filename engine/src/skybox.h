#pragma once

#include <gr-base/script/default.hpp>

#include <gr-render/gRender.h>

#include <functional>

#include "cubeFramebuffer.hpp"

class Skybox {
public:
    static void Initialize();

    static void Shutdown();

    static void Renderer(const Camera *camera);

    static grr::gTexture *GetTextureCubemap();

    static grr::gTexture *GetTextureIrradiance();

private:
    static grr::gVertexArray *m_vertexArray;

    static grr::gShader *m_irradianceShader;

    static grr::gShader *m_shader;

    static std::unique_ptr<CubeFramebuffer> m_colorbuffer;

    static std::unique_ptr<CubeFramebuffer> m_irradiance;

    static grr::gTexture *m_cubemapTexture;

    static grm::u32 m_count;

    static grm::u32 m_vbo;
    
    static grm::u32 m_ebo;

    static void CreateCube();
    
    static void ConfigCubemap(grr::gTexture *texture);

    static void CreateCubemapBackground(grr::gShader *shader);

    static void CreateCubemapIrradiance(grr::gShader *shader);

    static void GenerateMap(std::function<void(grm::u32)> func, grr::gTexture *texture, grr::gShader *shader);
};




