#include "entity.h"

#include "../scriptManager/scriptManager.hpp"

#include "../settings.h"

#include "script/meshRender.hpp"
#include "script/transform.hpp"

#include "core/scene.hpp"

// void (*Entity::Update)(Entity *);

Entity::Entity(GameObject *gameObject, EntityID id) : m_gameObject(gameObject), m_entityID(id) {}

GameObject *Entity::getGameObject() {
    return m_gameObject;
}

Component *Entity::addComponent(const std::string &name) {
    return addInternalComponent(name);
}

Component *Entity::getComponent(const std::string &name) {
    auto component = &(*m_componentPtr[name]);
    if (!component) {
        throw std::runtime_error("Component is inavlid.");
    }
    return component;
}

bool Entity::hasComponent(const std::string &name) const {
    auto it = m_componentPtr.find(name);
    return it != m_componentPtr.end();
}

std::unordered_map<std::string, Component *> &Entity::getComponents() {
    return m_componentPtr;
}

#include <list>
#include <tuple>

void Entity::copy(Entity *entity) {
    if (!entity) {
        throw std::runtime_error("Entity do not is valid.");
    }

    std::list<std::tuple<std::string, Component *>> m_list;
    for (auto &&[name, ptr] : entity->m_componentPtr) {
        m_list.emplace_front(name, &(*ptr));
    }

    for (auto &&[name, ptr] : m_list) {
        if (!ptr) {
            throw std::runtime_error("Component is not valid: " + name);
        }
        
        if (name == "Transform") {
            continue;
        }

        auto component = ScriptManager::CopyComponentWithName(name, ptr);
        if (!component) {
            throw std::runtime_error("Falied to copy component: " + name);
        }
        component->entity = this;
        component->Init();

        m_componentPtr.emplace(name, component);

        m_components.emplace_back(component);

        if (name == "MeshRender") {
            auto meshRender = dynamic_cast<MeshRender *>(component);
            meshRender->build();
        }
    }

    auto targetGameObject = entity->getGameObject();
    if (!targetGameObject) {
        throw std::runtime_error("GameObject is invalid");
    }

    auto &localTransform = m_gameObject->getTransform();

    auto &transform = targetGameObject->getTransform();
    localTransform.setLocalScale(transform.getLocalScale());
    localTransform.setLocalRotation(transform.getLocalRotation());
    localTransform.setLocalPosition(transform.getLocalPosition());

    if (!transform.hasParent()) {
        Scene::GetCurrentScene()->Add(getGameObject());
    } else {
        localTransform.setParent(transform.getParent());
    }

    for (auto &child : transform.getChildren()) {
        auto objectPtr = child->getGameObject();
        if (!objectPtr) {
            throw std::runtime_error("Child do not is valid of: " + targetGameObject->getName());
        }

        auto &clone = GameObject::Instantiate(objectPtr);
        auto &cTransform = clone.getTransform();
        cTransform.setParent(&localTransform);
    }
}

EntityID Entity::getEntityID() const {
    return m_entityID;
}

bool Entity::isActive() const {
    return m_gameObject->isActive();
}

void Entity::setEnable(bool value) {
    auto pages = (m_components.size() + MAX_PAGES - 1) / MAX_PAGES;
    for (auto i=0;i<pages;i++) {
        auto start = i * MAX_PAGES;
        auto end = std::min(start + MAX_PAGES, int(m_components.size()));

        for (auto j=start;j<end;j++) {
            m_components[j]->getGameObject()->setEnable(value);

            if (value) {
                m_components[j]->Enable();
                continue;
            }
            m_components[j]->Disable();
        }
    }
}

void Entity::init() {
    auto pages = (m_components.size() + MAX_PAGES - 1) / MAX_PAGES;
    for (auto i=0;i<pages;i++) {
        auto start = i * MAX_PAGES;
        auto end = std::min(start + MAX_PAGES, int(m_components.size()));

        for (auto j=start;j<end;j++) {
            if (m_components[j])
                m_components[j]->Init();
        }
    }
}

void Entity::clear() {
    m_componentPtr.clear();

    m_components.clear();
}

Component *Entity::addInternalComponent(const std::string &name) {
    if (!ScriptManager::CreateComponentWithName) {
        return nullptr;
    }

    auto component = ScriptManager::CreateComponentWithName(name);
    if (!component) {
        throw std::runtime_error("Failed to generate component for name: " + name);
    }
    
    component->entity = this;
    component->Init();

    m_componentPtr.emplace(name, component);

    m_components.emplace_back(component);

    return component;
}
