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

#include <linux/input-event-codes.h>

using namespace gr;

void CreateWindow(GParams* params) {
    params->window.title = "gr - egine";
    params->window.width = 1280;
    params->window.height = 800;
    params->window.x = 0;
    params->window.y = 0;
}

gTexture* m_texture = nullptr;

void Init() {
    int x, y, ch;
    auto pixels = stbi_load("/home/grsource/Downloads/16158812-eeeeeeee_xl.jpg", &x, &y, &ch, 0);

    m_texture = gTexture::Create(x, y, 0, pixels);

    stbi_image_free(pixels);
}

void Keyboard(const GKeyboard& keyboard) {
    if (keyboard.key == KEY_W && keyboard.action == BTN_GEAR_UP) {
        std::cout << " W  Up" << std::endl;
    }
    if (keyboard.key == KEY_W && keyboard.action == BTN_GEAR_DOWN) {
        std::cout << " W  down" << std::endl;
    }
    if (keyboard.key == KEY_W && keyboard.action == KEY_REDO) {
        std::cout << " W  PRESS" << std::endl;
    }
}

void MousePos(const GMouse& mouse) {
    std::cout << "X: " << mouse.x << " " << mouse.y << std::endl;
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

    Matrix4x4 model =
        Math::translate<Matrix4x4>(Vector3::zero) *
        Math::rotate<Matrix4x4>(Quaternion::identity) *
        Math::scale<Matrix4x4>(Vector3::one);

    Matrix4x4 projection = Math::perspective(Math::rad(90.0f), (float)(Screen::width / Screen::height), 1000.0f, 0.01f);
    Matrix4x4 view = Math::lookAt(Vector3(0.0f, 0.0f, -5.0f), Vector3::zero, Vector3::up);

    gRender::SetRenderState(GR_BACKGROUND_COLOR, (void*)&Color::black);
    gRender::SetRenderState(GR_BACKGROUND, GR_COLOR_BUFFER | GR_DEPTH_BUFFER);

    gRender::Render3D(projection, view);

    std::vector<Vertex3D> vertices;

    for (const auto& v : vertex3D) {
        vertices.push_back({Vector3(model * Vector4(v.position, 1.0f))});
    }

    gRender::RenderIndexedPrimitive3D(PrimitiveType_Triangles, vertices.size(), vertices.data(), indices3D.size(), indices3D.data());
}

void Destroy() {
    std::cout << "Engine Destroy." << std::endl;
}

void* HandleEvent(Engine::Event event, GParams* params) {
    switch (event) {
    case Engine::GR_CREATEWINDOW:
        CreateWindow(params);
        break;
    case Engine::GR_INITIALIZE:
        Init();
        break;
    case Engine::GR_UPDATE:
        Update();
        break;
    case Engine::GR_RENDERER:
        Renderer();
        break;
    case Engine::GR_KEYBOARD: {
        const GKeyboard& keyboard = params->keyboard;
        
        Keyboard(keyboard);
        break;
    }
    case Engine::GR_MOUSE_POS: {
        const GMouse& mouse = params->mouse;
        MousePos(mouse);
        break;
    }
    case Engine::GR_DESTROY:
        Destroy();
        break;
    case Engine::GR_ERROR:
        std::cout << "Engine error: " << Engine::GetError() << std::endl;
        break;
    default:
        break;
    }
    return nullptr;
}

int main() {
    if (!Engine::Initialize(HandleEvent)) {
        std::cout << "Falha ao iniciar a engine." << std::endl;
        return 1;
    }
    return 0;
}

