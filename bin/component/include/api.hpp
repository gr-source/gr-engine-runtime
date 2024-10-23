#pragma once

#include "script/default.hpp"
#include "core/console.hpp"

#include "ecs/component.hpp"

#include <gr-render/gRender.h>

#define GR_EXPORT __attribute__((visibility("default")))

#ifdef __cplusplus
extern "C" {
#endif

// [ Initialzie ]
void Initialize();

void Destroy();

void SetDeltaTime(float deltaTime);

// [ Component - Camera ]
void SetEnableCamera(Camera *camera);

std::vector<Camera *> &GetAllCamera();

// Light
std::vector<Light*> &GetListLight();

// AssetsManager
grr::gShader *LoadShader(const std::string &frag, const std::string &vert);

Animation *LoadAnimation(const std::string &path);

// Screen
void SetScreenSize(const iVector2 &value);

// Component

std::unordered_map<std::string, std::unique_ptr<Pool>> &GetComponentPool();

Component *CreateComponentWithName(const std::string &name);

Component *CopyComponentWithName(const std::string &name, const Component *component);

bool InputIsKeyPress(grm::u32 key);

void MaterialDeserialize(Material *instance, Stream *stream);

void AssetsManagerSetShaderList(const std::string &name, const std::string &frag, const std::string &vert, std::function<void(grr::gShader *)> func);

#ifdef __cplusplus
}
#endif



