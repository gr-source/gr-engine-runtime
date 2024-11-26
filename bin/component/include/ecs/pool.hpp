#pragma once

#include "field.hpp"

#include <functional>

enum class FuncType {
    UPDATE,
    INIT,
    DESTROY,
};

class Pool {
public:
    virtual ~Pool() {}

    virtual Component *CreateInstance(const Component *component) = 0;

    virtual void Instance(Component *component) = 0;

    template <typename ...Args>
    void RegisterField(const std::string &name, Args &&...args);

    void (*update)(void *);

private:
    std::unordered_map<std::string, grm::size> m_fieldID;

    std::vector<Field> m_fieldMap;
};

template <typename... Args>
inline void Pool::RegisterField(const std::string &name, Args &&...args) {
    auto it = m_fieldID.find(name);
    if (it != m_fieldID.end()) {
        return;
    }

    auto id = m_fieldMap.size();
    m_fieldMap.emplace_back(name, std::forward<Args>(args)...);
    m_fieldID.emplace(name, id);
}



