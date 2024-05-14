#pragma once

#include <utility>
#include <string>

#include "ecs/Component.h"
#include "ecs/Entity.h"

#include "component/Transform.h"

class GameObject {
private:
    Entity* entity;
public:
    /* nome do objeto */
    std::string name;

    /* defini um grupo de objetos */
    std::string tag;

    /* objeto statico verifica se o objeto vai se mover na cena */
    bool isStatic;

    /* deletar o objeto na proxima verificaçao */
    bool isDelete;

    GameObject(const std::string& name = "none", const std::string& tag = "none");
    ~GameObject();

    const std::string& getname() const;

    const std::string& gettag() const;

    /* adiciono um componenet ao objeto */
    template <typename T, typename ...Argv>
    T& addComponent(Argv&&... argv);

    /* pegue um component do objeto */
    template <typename T>
    T& getComponent();

    /* verifique de o component existe nesse objeto */
    template <typename T>
    const bool hasComponent() const;

    Transform& gettransform();

    template <typename T, typename Callback, typename... Args>
    void RenderProperty(Callback&& callback, Args&&... args);

    bool operator ==(const GameObject& other) const;
};

template <typename T, typename ...Argv>
inline T& GameObject::addComponent(Argv&&... argv) {
    return entity->addComponent<T>(std::forward<Argv>(argv)...);
}

template <typename T>
inline T& GameObject::getComponent() {
    return entity->getComponent<T>();
}

template <typename T>
inline const bool GameObject::hasComponent() const {
    return entity->hasComponent<T>();
}

template <typename T, typename Callback, typename... Args>
inline void GameObject::RenderProperty(Callback&& callback, Args&&... args) {
    if (!entity->hasComponent<T>()) return;

    std::bind(std::forward<Callback>(callback), std::forward<Args>(args)..., &entity->getComponent<T>())();
}
