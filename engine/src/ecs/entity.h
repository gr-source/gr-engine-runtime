#pragma once

#include <unordered_map>
#include <algorithm>
#include <stdexcept>
#include <memory>
#include <vector>
#include <bitset>
#include <array>

#include "component.h"

#define MAX_COMPONENT 16

class GameObject;

/* tamanho maximo de componenets */
constexpr std::size_t maxComponent = 32;

/* gere um id para */
inline std::size_t GenerateID() noexcept {
    static std::size_t id = 0;
    return ++id;
}

/* Gere um id apartir de um componenet */
template <typename T>
inline std::size_t GetIDFromComponent() noexcept {
    static std::size_t id = GenerateID();
    return id;
}

/* Classe de entidades */
class Entity {
public:
    static std::size_t GetEntityCount();

    static void Process();

    template <typename T>
    T& getComponent();

    GameObject *getGameObject();
private:
    std::unordered_map<std::string, std::shared_ptr<Component>> components;

    std::bitset<MAX_COMPONENT> sequence;

    GameObject *gameObject;
};

template <typename T>
inline T &Entity::getComponent() {
    return *dynamic_cast<T*>(components[typeid(T).name()].get());
}

