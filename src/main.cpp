#include <iostream>

#include <engine.h>
#include <Color.h>

#include <plataform/gl/grende.h>


void Initilialize() {
    Monitor& monitor = Engine::monitor;
    monitor.title = "gr - egine";
    monitor.width = 1280;
    monitor.height = 800;
    monitor.x = 0;
    monitor.y = 0;
}

void Update() {}

void Renderer() {
    SetRenderState(GR_BACKGROUND_COLOR, (void*)&Color::white);
    SetRenderState(GR_BACKGROUND, GR_COLOR_BUFFER);
}

void Destroy() {
    std::cout << "Engine Destroy." << std::endl;
}

void HandleEvent(Engine::Event event) {
    switch (event) {
    case Engine::GRINITIALIZE:
        Initilialize();
        break;
    case Engine::GRUPDATE:
        Update();
        break;
    case Engine::GRRENDERER:
        Renderer();
        break;
    case Engine::GRDESTROY:
        Destroy();
        break;
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

