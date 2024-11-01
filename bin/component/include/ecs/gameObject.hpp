#pragma once

#include "../script/default.hpp"

class GameObject {
public:
    static GameObject &Create(const std::string &name, const std::string &tag);
    
    static GameObject &Instantiate(GameObject *target);

    static void Destroy(GameObject *target);

    GameObject() = delete;
    ~GameObject() = delete;

    void setName(const std::string &name);

    const std::string& getName() const;

    const std::string& getTag() const;

    void setEnable(bool value);

    void setActive(bool value);

    bool isActive() const;

    template <typename T>
    T *addComponent();

    template <typename T>
    T *getComponent();

    template <typename T>
    bool hasComponent() const;

    Transform *getTransform();

    Entity *GetEntity();

    GameObject *getChild(int idx);

private:
    Transform *m_transform;
    
    std::string m_name;

    std::string m_tag;

    Entity* m_entity;

    bool m_bActive;

    bool m_bEnable;
};

#include "entity.hpp"

template <typename T>
inline T *GameObject::addComponent() {
    return m_entity->addComponent<T>();
}

template <typename T>
inline T *GameObject::getComponent() {
    return m_entity->getComponent<T>();
}

template <typename T>
inline bool GameObject::hasComponent() const {
    return m_entity->hasComponent<T>();
}
