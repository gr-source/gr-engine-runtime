#pragma once

#include <algorithm>
#include <memory>
#include <vector>
#include <array>

#include "Component.h"

#include "../renderer.h"

class EntityManager;

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
    /// @brief lista de components, quardad todos os components na lista
    std::array<std::shared_ptr<Component>, maxComponent> components;

    /* script esta abilitado (Função da game engine nao vai pro jogo final)*/
    bool IsActivate;

    /* construto da classe */
    Entity();

    /// @brief Adiciona um component a entidade atual
    /// @tparam T Script classe eradde de component
    /// @tparam ...Args tipos da onstrutor da classe
    /// @param ...args argumentos a ser passado a classe do script
    /// @return retorna o script
    template <typename T, typename... Args>
    T& addComponent(Args&& ... args);

    /// @brief Pega um component da entidade atual
    /// @tparam T a classe script desejada
    /// @return retorna o a referencia da da classe script
    template <typename T>
    T& getComponent();

    /* função pra verificar se o component existe */
    template <typename T>
    bool hasComponent() const;

    /* Função pra iniciar as script */
    void OnStart();

    /* função pra atualizar a as scripts */
    void OnUpdate();

    /* Renderize as scripts */
    void OnRender(DebugDraw& debugDraw);

    /* UI property engine */
    void OnGui();

    void OnCollidedWith(const GameObject& other);

    /* operador pra verificar se duas entidades sao iguais */
    bool operator==(const Entity& other) const;
};

template <typename T, typename... Args>
inline T& Entity::addComponent(Args&&... args) {
    if (hasComponent<T>()) {
        return getComponent<T>();
    }
    
    auto& c = components[GetIDFromComponent<T>()] = std::make_shared<T>(std::forward<Args>(args)...);

    c->entity = this;

    c->OnInit();

    #if IS_GAME
    c->OnStart();
    #endif

    return *reinterpret_cast<T*>(c.get());
}

template <typename T>
inline T& Entity::getComponent() {
    return *reinterpret_cast<T*>(components[GetIDFromComponent<T>()].get());
}

template <typename T>
inline bool Entity::hasComponent() const {
    return components[GetIDFromComponent<T>()] != nullptr;
}



