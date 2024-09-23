#pragma once

#include "default.hpp"

class Field {
public:
    Field(const std::string &name = "default", const std::string &type = "undefined", grm::size offset = 0);

private:
    std::string m_name;

    std::string m_type;

    grm::size m_offset;
};

