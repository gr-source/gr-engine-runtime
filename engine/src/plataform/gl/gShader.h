#pragma once

#include <vector>
#include <string>

#include "types.h"

class gShader {
public:
    gShader();

    static gShader* Create(const std::vector<std::string>& fragments, const std::vector<std::string>& vertex);

    void destroy();

private:
    u32 m_index;

    static gShader* instance;
};

