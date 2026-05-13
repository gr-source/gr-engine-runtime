#include <iostream>

#include <gr-base/core/logger.hpp>

__attribute__((constructor))
static void Initialize()
{
    logger::info("[Script] Initialize.");
}

__attribute__((destructor))
static void Shutdown()
{
    logger::info("[Script] Shutdown.");
}

