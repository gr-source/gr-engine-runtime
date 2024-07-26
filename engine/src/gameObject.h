#pragma once

#include <string>

#include "ecs/entity.h"

#include "component/Transform.h"

class GameObject {
public:
    const std::string &getName();

    const std::string &getTag();

    Transform &getTransform();

    template <typename T>
    T& getComponent();
    
private:
    std::string m_name;

    std::string m_tag;

    Entity* m_entity;

protected:
    static std::vector<std::unique_ptr<GameObject>> s_objects;
};

template <typename T>
inline T& GameObject::getComponent() {
    return m_entity->getComponent<T>();
}

/*
class GameObject {
private:
    Entity* entity;
public:
    /* nome do objeto 
    std::string name;

    /* defini um grupo de objetos 
    std::string tag;

    GameObject(const std::string& name = "none", const std::string& tag = "none");
    ~GameObject();

    const std::string& getname() const;

    const std::string& gettag() const;

    /* adiciono um componenet ao objeto 
    template <typename T, typename ...Argv>
    constexpr T& addComponent(Argv&&... argv);

    /* pegue um component do objeto 
    template <typename T>
    T& getComponent();

    /* verifique de o component existe nesse objeto 
    template <typename T>
    constexpr bool hasComponent() const;

    Transform& getTransform();

    template <typename T, typename... Args>
    void RenderProperty(Args&&... args);

    bool operator ==(const GameObject& other) const;
};

template <typename T, typename ...Argv>
inline constexpr T& GameObject::addComponent(Argv&&... argv) {
    return entity->addComponent<T>(std::forward<Argv>(argv)...);
}

template <typename T>
inline T& GameObject::getComponent() {
    return entity->getComponent<T>();
}

template <typename T>
inline constexpr bool GameObject::hasComponent() const {
    return entity->hasComponent<T>();
}

template <typename T, typename... Args>
inline void GameObject::RenderProperty(Args&&... args) {
    if (!entity->hasComponent<T>()) return;

    std::bind(std::forward<Args>(args)..., &entity->getComponent<T>())();
}
*/