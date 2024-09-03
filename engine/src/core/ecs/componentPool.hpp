#pragma once

#include <unordered_map>
#include <vector>
#include <string>

#include <gr-math/gmath.h>

#include "ecs/component.h"

class Field {
public:
    const std::string &getName() const;

    const std::string &getType() const;

    const grm::size &getOffset() const;

    template <typename T>
    static T *GetFieldData(void *ptr, const Field &field);

private:
    std::string m_name;
    
    std::string m_type;

    grm::size m_offset;
};

class Pool {
public:
    virtual Component *CreateInstanceFrom(const Component *component) = 0;

    virtual Component *CreateInstance() = 0;

    std::vector<Field> &getFields();

private:
    std::unordered_map<std::string, grm::size> m_fieldID;

    std::vector<Field> m_fieldMap;
};

template <typename T>
inline T *Field::GetFieldData(void *ptr, const Field &field) {
    return reinterpret_cast<T *>(reinterpret_cast<grm::u8 *>(ptr) + field.getOffset());
}
