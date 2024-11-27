#include "api.hpp"

// #include "script/meshRender.hpp"
#include "script/camera.hpp"
#include "script/light.hpp"

// #include "core/assetsManager.hpp"
#include "core/screen.hpp"
// #include "core/scene.hpp"
#include "core/event.hpp"
#include "core/time.hpp"

void SetEnableCamera(Camera *camera) {
    return Camera::SetEnable(camera);
}

std::vector<Camera *> &GetAllCamera() {
    return Camera::s_camera;
}

std::vector<Light *> &GetListLight() {
    return Light::m_lights;
}

grr::gShader *LoadShader(const std::string &frag, const std::string &vert) {
    return nullptr;
}

Animation *LoadAnimation(const std::string &path) {
    return nullptr;
}

grr::gShader *LoadShaderSTRSTR(const std::string &frag, const std::string &vert) {
    return nullptr;
}

void SetScreenSize(const iVector2 &value) {
    Screen::ScreenSize = value;
}

std::unordered_map<std::string, Pool *> GetComponentPool() {
    return ComponentRegistry::GetPool();
}

/*
Component *CreateInternalComponent(const std::string &name, const Component *component) {
    auto pool = ComponentRegistry::Find(name);
    if (!pool) {
        throw std::runtime_error("Falied to create component: " + name + ", component is not valid.");
    }
    return pool->CreateInstance(component);
}

void ReinstanceComponent(const std::string &name, Component *component) {
    auto pool = ComponentRegistry::Find(name);
    if (!pool) {
        throw std::runtime_error("Falied to create component: " + name + ", component is not valid.");
    }
    return pool->Instance(component);
}
*/

void SetDeltaTime(float deltaTime) {
    Time::deltaTime = deltaTime;
}



