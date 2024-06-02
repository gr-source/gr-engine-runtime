#include <iostream>
#include <vector>

#include <engine.h>
#include <screen.h>
#include <timer.h>
#include <Color.h>

#include <gTexture.h>
#include <gRender.h>

#include <imgui.h>

#include <core/stb/stb_image.h>

using namespace gr;

void Init() {
    Monitor& monitor = Plataform::GetMonitor();
    monitor.title = "gr - egine";
    monitor.width = 1280;
    monitor.height = 800;
    monitor.x = 0;
    monitor.y = 0;
}

gTexture* m_texture = nullptr;

void InitOpenGL() {

    int x, y, ch;
    auto pixels = stbi_load("/home/grsource/Downloads/16158812-eeeeeeee_xl.jpg", &x, &y, &ch, 0);

    m_texture = gTexture::Create(x, y, 0, pixels);
}

void Update() {}

std::vector<Vertex3D> vertex3D {
    {Vector3(-1.0f, -1.0f,  1.0f)},
    {Vector3(-1.0f,  1.0f,  1.0f)},
    {Vector3(-1.0f, -1.0f, -1.0f)},
    {Vector3(-1.0f,  1.0f, -1.0f)},
    {Vector3( 1.0f, -1.0f,  1.0f)},
    {Vector3( 1.0f,  1.0f,  1.0f)},
    {Vector3( 1.0f, -1.0f, -1.0f)},
    {Vector3( 1.0f,  1.0f, -1.0f)}
};

std::vector<u32> indices3D {
    1, 2, 0,
    3, 6, 2,
    7, 4, 6,
    5, 0, 4,
    6, 0, 2,
    3, 5, 7,
    1, 3, 2,
    3, 7, 6,
    7, 5, 4,
    5, 1, 0,
    6, 4, 0,
    3, 1, 5
};

std::vector<Vertex2D> vertex2D {
    {Vector2(-1.0f,  1.0f), Vector2(1.0f, 0.0f)},
    {Vector2( 1.0f,  1.0f), Vector2(0.0f, 1.0f)},
    {Vector2(-1.0f, -1.0f), Vector2(0.0f, 0.0f)},
    {Vector2( 1.0f, -1.0f), Vector2(1.0f, 1.0f)}
};

std::vector<u32> indices2D {
    1, 2, 0,
    1, 3, 2
};

void Renderer() {
    Vector2 centerScreen(Screen::width / 2.0f, Screen::height / 2.0f);

    Matrix3x3 model =
        Math::translate<Matrix3x3>(centerScreen) *
        Math::rotate<Matrix3x3>(Quaternion::identity) *
        Math::scale<Matrix3x3>(Vector2(32, 32));


    gRender::SetRenderState(GR_BACKGROUND_COLOR, (void*)&Color::black);
    gRender::SetRenderState(GR_BACKGROUND, GR_COLOR_BUFFER);

    gRender::Render2D();

    gRender::RenderTexture2D(model, m_texture);
}

void Destroy() {
    std::cout << "Engine Destroy." << std::endl;
}

void HandleEvent(Engine::Event event) {
    switch (event) {
    case Engine::GRINITIALIZE:
        return Init();
    case Engine::GRINITIALIZEOPENGL:
        return InitOpenGL();
    case Engine::GRUPDATE:
        return Update();
    case Engine::GRRENDERER:
        return Renderer();
    case Engine::GRDESTROY:
        return Destroy();
    case Engine::GRERROR:
        std::cout << "Engine error: " << Engine::GetError() << std::endl;
        break;
    default:
        break;
    }
}

int main() {
    if (!Engine::Initialize(HandleEvent)) {
        std::cout << "Falha ao iniciar a engine." << std::endl;
        return 1;
    }
    return 0;
}

