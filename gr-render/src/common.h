#pragma once

<<<<<<< HEAD
#include <unordered_map>
#include <iostream>
#include <string>
#include <vector>

#include <gr-math/gmath.h>

#include "types.h"

namespace gr {
    class gTexture;
    class gRender;
    class gShader;
};


=======
#include <stdexcept>
#include <cstdint>
#include <cstdlib>

#define GCALLOC(sz, ptr) std::calloc(sz, ptr)
#define GMALLOC(sz) std::malloc(sz)
#define GFREE(ptr) std::free(ptr)

#define GEXCEPTION(str) std::runtime_error(str)

namespace grr {
    using u8 = std::uint8_t;

    using u16 = std::uint16_t;

    using u32 = std::uint32_t;

    using ptrsize = std::ptrdiff_t;
};
>>>>>>> 7c9a1b2 (	new file:   .gitignore)
