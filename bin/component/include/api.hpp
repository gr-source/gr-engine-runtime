#pragma once

#include "script/default.hpp"
#include "core/console.hpp"

#include <gr-render/gRender.h>

#define GR_EXPORT __attribute__((visibility("default")))

#ifdef __cplusplus
extern "C" {
#endif

// [ Initialzie ]
void Initialize();

void Destroy();

void SetDeltaTime(float deltaTime);

std::vector<Camera *> &GetAllCamera();

// Light
std::vector<Light*> &GetListLight();

// AssetsManager
grr::gShader *LoadShader(const std::string &frag, const std::string &vert);

Animation *LoadAnimation(const std::string &path);

// Screen
void SetScreenSize(const iVector2 &value);


#ifdef __cplusplus
}
#endif



