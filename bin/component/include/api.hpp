#pragma once

#include <gr-base/script/default.hpp>

#include <gr-render/gRender.h>

__attribute__((constructor))
static void initialize();

__attribute__((destructor))
static void shutdown();

