#pragma once

#include "default.hpp"

#include <unordered_map>
#include <memory>

class GameObject {
public:
    static GameObject *Create(const std::string& name, const std::string& tag = "unttaged");

    static GameObject *Instantiate(GameObject *target);

    static GameObject *Find(const std::string &name);

    static void Rename(GameObject *gameObject, std::string name);

    static void Destroy(GameObject *target);

    static grm::size GetGameObjectCount();

    static void Clear();

    GameObject() = delete;
    GameObject(const std::string& name, const std::string& tag = "unttaged");
    ~GameObject();

    void initialize();

    std::string getName() const;

    void setName(const std::string &name);

    std::string getTag() const;

    void setActive(bool value);
    
    bool isActive() const;

    void setTransform(Transform *transform);

    Transform *getTransform() const;

    template <typename T>
    T *addComponent();

    template <typename T>
    T *getComponent();

    template <typename T>
    bool hasComponent() const;

    template <typename T, typename... Args>
    bool RenderProperty(Args&&... args);

    Entity *getEntity() const;

    void setEnable(bool value);

    bool getActive() const;
    
    bool getEnable() const;
    
private:
    Transform *m_transform;
    
    std::string m_name;

    std::string m_tag;

    Entity* m_entity;

    bool m_bActive;

    bool m_bEnable;

protected:
    static std::unordered_map<std::string, std::unique_ptr<GameObject>> m_dataObject;
};

#include "entity.h"

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
    return m_entity ? m_entity->hasComponent<T>() : false;
}

template <typename T, typename... Args>
inline bool GameObject::RenderProperty(Args&&... args) {
    if (!m_entity->hasComponent<T>()) return false;

    return std::invoke(std::bind(std::forward<Args>(args)..., m_entity->getComponent<T>()));
}
