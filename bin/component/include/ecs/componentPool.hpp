#pragma once

#include "pool.hpp"

template <typename T>
class ComponentPool : public Pool {
public:
    Component *CreateInstance(const Component *component) override;

    void Instance(Component *component) override;
};

template <typename T>
inline Component *ComponentPool<T>::CreateInstance(const Component *component) {
    auto c = component != nullptr ? new T(*dynamic_cast<const T *>(component)) : new T();
    if (!c) {
        throw std::runtime_error("Falied on create istance.");
    }

    return dynamic_cast<Component *>(c);
}

template <typename T>
inline void ComponentPool<T>::Instance(Component *component) {
    new (component) T(*static_cast<T *>(component));
    // auto c = component != nullptr ? new T(*dynamic_cast<const T *>(component)) : new T();
    // if (!c) {
        // throw std::runtime_error("Falied on create istance.");
    // }

    // return dynamic_cast<Component *>(c);
}
