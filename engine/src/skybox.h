#pragma once

#include <gr-render/gRender.h>

#include <functional>

class CubeFramebuffer;

class Skybox {
public:
    static void Initialize();

    static void Shutdown();

    static void Renderer();

    static grr::gTexture *GetTextureCubemap();

private:
    static grr::gVertexArray *m_vertexArray;

    static grr::gShader *m_irradianceShader;

    static grr::gShader *m_skyShader;

    static grr::gShader *m_shader;

    static CubeFramebuffer *m_cubeColor;

    static grr::gTexture *m_texture;

    static grm::u32 m_count;

    static grm::u32 m_vbo;
    
    static grm::u32 m_ebo;

    static void CreateCube();
    
    static bool SetupTexture(void *texture, const std::string &path);

    static void UploadShader();

    static void RegisterSky(grr::gShader **out, grr::gShader *shader);

    static void GenerateMap(std::function<void(grm::u32)> func, grr::gTexture *texture);
};




