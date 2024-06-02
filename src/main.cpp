#include <iostream>
#include <vector>

#include <engine.h>
#include <timer.h>
#include <Color.h>

#include <plataform/gl/gRender.h>

using namespace gr;

void Init() {
    Monitor& monitor = Engine::monitor;
    monitor.title = "gr - egine";
    monitor.width = 1280;
    monitor.height = 800;
    monitor.x = 0;
    monitor.y = 0;
}

void InitOpenGL() {
}

void Update() {}

std::vector<Vertex3D> vertex {
    {Vector3(-1.0f, -1.0f,  1.0f)},
    {Vector3(-1.0f,  1.0f,  1.0f)},
    {Vector3(-1.0f, -1.0f, -1.0f)},
    {Vector3(-1.0f,  1.0f, -1.0f)},
    {Vector3( 1.0f, -1.0f,  1.0f)},
    {Vector3( 1.0f,  1.0f,  1.0f)},
    {Vector3( 1.0f, -1.0f, -1.0f)},
    {Vector3( 1.0f,  1.0f, -1.0f)}
};

std::vector<u32> indices {
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

void Renderer() {

    std::vector<Vertex3D> vertices;

    Matrix4x4 model =
        Math::translate<Matrix4x4>(Vector3::zero) *
        Math::rotate<Matrix4x4>(Quaternion::identity) *
        Math::scale<Matrix4x4>(Vector3::one);

    for (const auto& v : vertex) {
        vertices.push_back({Vector3(model * Vector4(v.position))});
    }

    gRender::SetRenderState(GR_BACKGROUND_COLOR, (void*)&Color::black);
    gRender::SetRenderState(GR_BACKGROUND, GR_COLOR_BUFFER | GR_DEPTH_BUFFER);

    gRender::Render3D();

    gRender::RenderIndexedPrimitive3D(PrimitiveType_Triangles, vertices.size(), vertices.data(), indices.size(), indices.data());

    /*gRender::SetRenderState(GR_BACKGROUND_COLOR, (void*)&Color::black);
    gRender::SetRenderState(GR_BACKGROUND, GR_COLOR_BUFFER);

    gRender::Render2D();

    gRender::RenderPrimitive2D(PrimitiveType_LineLoop, vertices.size(), vertices.data());*/
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

