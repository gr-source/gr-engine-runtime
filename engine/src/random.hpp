#pragma once

#include "common.h"

class Random {
public:
    static void SetSeed(ulong seed);

    template <typename T>
    static T Rand(T min, T max);
    
private:
    static ulong s_seed;
};

template <typename T>
inline T Random::Rand(T min, T max) {
    return ((float)std::rand() / RAND_MAX) * (max - min) + min;
}
