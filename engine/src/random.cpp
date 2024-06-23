#include "random.hpp"

ulong Random::s_seed = 0;

void Random::SetSeed(ulong seed) {
    s_seed = seed;

    std::srand(s_seed);
}
