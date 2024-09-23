#pragma once

#include "pool.hpp"

template <typename T>
class ComponentPool : public Pool {
public:
    Component *CreateInstanceFrom(const Component *component) override;

    Component *CreateInstance() override;
};

template <typename T>
inline Component *ComponentPool<T>::CreateInstance() {
    auto component = new T();
    if (!component) {
        throw std::runtime_error("Falied to create istance.");
    }
    return dynamic_cast<Component *>(component);
}

template <typename T>
inline Component *ComponentPool<T>::CreateInstanceFrom(const Component *component) {
    if (!component) {
        throw std::runtime_error("Target component is invalid.");
    }

    auto c = new T(*dynamic_cast<const T *>(component));
    if (!component) {
        throw std::runtime_error("Falied on create istance.");
    }

    return dynamic_cast<Component *>(c);
}



