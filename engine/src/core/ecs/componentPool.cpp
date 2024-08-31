#include "componentPool.hpp"

const std::string & Field::getName() const {
    return m_name;
}

const std::string &Field::getType() const {
    return m_type;
}

const grm::size &Field::getOffset() const {
    return m_offset;
}

std::vector<Field> &Pool::getFields() {
    return m_fieldMap;
}
