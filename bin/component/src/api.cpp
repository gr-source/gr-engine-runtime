#include "api.hpp"

// #include "script/meshRender.hpp"
#include "script/camera.hpp"
#include "script/light.hpp"

// #include "core/assetsManager.hpp"
// #include "core/screen.hpp"
// #include "core/scene.hpp"
#include "core/event.hpp"
#include "core/time.hpp"

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

void SetDeltaTime(float deltaTime) {
    Time::deltaTime = deltaTime;
}



