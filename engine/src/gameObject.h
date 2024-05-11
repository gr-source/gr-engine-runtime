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
    T& addComponent(Argv&&... argv) const;

    /* pegue um component do objeto */
    template <typename T>
    T& getComponent() const;

    /* verifique de o component existe nesse objeto */
    template <typename T>
    bool hasComponent() const;

    Transform& gettransform();

    void OnGui();

    bool operator ==(const GameObject& other) const;
};

template <typename T, typename ...Argv>
inline T& GameObject::addComponent(Argv&&... argv) const {
    return entity->addComponent<T>(std::forward<Argv>(argv)...);
}

template <typename T>
inline T& GameObject::getComponent() const {
    return entity->getComponent<T>();
}

template <typename T>
inline bool GameObject::hasComponent() const {
    return entity->hasComponent<T>();
}


