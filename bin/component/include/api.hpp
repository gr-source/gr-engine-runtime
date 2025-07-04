#pragma once

#include <gr-base/script/default.hpp>

#include <gr-render/gRender.h>

#define GR_EXPORT __attribute__((visibility("default")))

#ifdef __cplusplus
extern "C" {
#endif

// [ Initialzie ]
void Initialize();

void Destroy();

void SetDeltaTime(float deltaTime);

#ifdef __cplusplus
}
#endif



