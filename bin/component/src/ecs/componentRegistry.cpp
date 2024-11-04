#include "ecs/componentRegistry.hpp"

ComponentRegistry &ComponentRegistry::getInstance() {
    static ComponentRegistry m_instance;
    return m_instance;
}

Pool *ComponentRegistry::Find(const std::string &name) {
    auto it = getInstance().m_poolMap.find(name);
    if (it == getInstance().m_poolMap.end()) {
        return nullptr;
    }
    return &(*it->second);
}

std::unordered_map<std::string, Pool *> ComponentRegistry::GetPool() {
    return getInstance().m_poolMap;
}



