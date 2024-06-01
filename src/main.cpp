#include <iostream>

#include <engine.h>
#include <Color.h>

#include <plataform/gl/grende.h>
#include <plataform/gl/gShader.h>

std::string fragment = R"(
    #version 330 core

    out vec4 FragColor;

    void main() {
        FragColor = vec4(1.0);
    }
)";

std::string vertex = R"(
    #version 330 core

    layout (location = 0) in vec2 in_pos;

    uniform mat4 projection;
    uniform mat3 model;

    void main() {
        gl_Position = projection * vec4(model * vec3(in_pos, 1.0), 1.0);
    }
)";


void Init() {
    Monitor& monitor = Engine::monitor;
    monitor.title = "gr - egine";
    monitor.width = 1280;
    monitor.height = 800;
    monitor.x = 0;
    monitor.y = 0;
}

gShader* m_shader = nullptr;

void InitOpenGL() {
    m_shader = gShader::Create({fragment}, {vertex});
}

void Update() {}

void Renderer() {
    SetRenderState(GR_BACKGROUND_COLOR, (void*)&Color::white);
    SetRenderState(GR_BACKGROUND, GR_COLOR_BUFFER);
}

void Destroy() {
    if (m_shader) {
        m_shader->destroy();
    }
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

