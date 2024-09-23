#include "ecs/field.hpp"

Field::Field(const std::string &name, const std::string &type, grm::size offset) : m_name(name), m_type(type), m_offset(offset) {}


