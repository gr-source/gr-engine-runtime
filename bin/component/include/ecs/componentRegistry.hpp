#pragma once

#include "componentPool.hpp"

#include <memory>

class ComponentRegistry {
public:
    static ComponentRegistry &getInstance();

    static Pool *Find(const std::string &name);

    static std::unordered_map<std::string, std::unique_ptr<Pool>> &GetPool();

    template <typename T>
    bool RegisterComponent();

    template <typename T>
    ComponentPool<T> *GetComponentPool();

private:
    std::unordered_map<std::string, std::unique_ptr<Pool>> m_poolComponent;
};

#include "core/string.hpp"

template <typename T>
inline bool ComponentRegistry::RegisterComponent() {
    auto name = StringUTF::RemoveNumber(typeid(T).name());

    auto it = m_poolComponent.find(name);
    if (it != m_poolComponent.end()) {
        return true;
    }
    auto &&[iterator, added] = m_poolComponent.emplace(name, std::make_unique<ComponentPool<T>>());
    if (!added) {
        throw std::runtime_error("Component: " + name + " replaced.");
    }
    return true;
}

template <typename T>
inline ComponentPool<T> *ComponentRegistry::GetComponentPool() {
    auto it = m_poolComponent.find(StringUTF::RemoveNumber(typeid(T).name()));
    if (it == m_poolComponent.end()) {
        throw std::runtime_error("Component do not added: " + std::string(typeid(T).name()));
    }
    auto ptr = &(*it->second);
    if (!ptr) {
        throw std::runtime_error("Ivalid component pointer: " + std::string(typeid(T).name()));
    }

    auto typedPool = dynamic_cast<ComponentPool<T> *>(ptr);
    if (!typedPool) {
        throw std::runtime_error("Error in type cast.");
    }
    return typedPool;
}
