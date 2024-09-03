#pragma once

#include "default.hpp"

#include "entity.h"

class GameObject {
public:
    static GameObject& Create(const std::string& name, const std::string& tag = "unttaged");

    static GameObject &Instantiate(GameObject *target);

    static GameObject *Find(const std::string &name);

    static void Rename(GameObject *gameObject, std::string name);

    static void Destroy(GameObject *target);

    static grm::size GetGameObjectCount();

    static void Clear();

    GameObject() = delete;
    GameObject(const std::string& name, const std::string& tag = "unttaged");
    ~GameObject();

    const std::string &getName();

    void setName(const std::string &name);

    const std::string &getTag();

    void setActive(bool value);
    
    bool isActive() const;

    Transform &getTransform();

    template <typename T>
    T &addComponent();

    template <typename T>
    T& getComponent();

    template <typename T>
    bool hasComponent() const;

    template <typename T, typename... Args>
    void RenderProperty(Args&&... args);

    Entity *getEntity();

    void setEnable(bool value);
    
private:
    Transform *m_transform;

    std::string m_name;

    std::string m_tag;

    Entity* m_entity;

    bool m_bActive;

    bool m_bEnable;
protected:

    static std::unordered_map<std::string, std::unique_ptr<GameObject>> s_mapObject;

    static std::unordered_map<std::string, int> s_names;

    static std::string AddNameID(const std::string &name);

    static void RemoveNameID(const std::string &name);
};

template <typename T>
inline T &GameObject::addComponent() {
    return m_entity->addComponent<T>();
}

template <typename T>
inline T &GameObject::getComponent() {
    return m_entity->getComponent<T>();
}

template <typename T>
inline bool GameObject::hasComponent() const {
    return m_entity->hasComponent<T>();
}

template <typename T, typename... Args>
inline void GameObject::RenderProperty(Args&&... args) {
    if (!m_entity->hasComponent<T>()) return;

    std::bind(std::forward<Args>(args)..., &m_entity->getComponent<T>())();
}
