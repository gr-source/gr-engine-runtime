#include "entity.h"

#include "../scriptManager/scriptManager.hpp"

#include "../settings.h"

#include "script/meshRender.hpp"
#include "script/transform.hpp"

#include "core/scene.hpp"

void (*Entity::Update)(Entity *);

void (*Entity::Init)(Entity *);

void (*Entity::SetEnable)(Entity *, bool);

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

std::unordered_map<std::string, std::shared_ptr<Component>> &Entity::getComponents() {
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

        m_componentPtr.emplace(name, std::shared_ptr<Component>(component));

        m_components.push_back(component);

        if (name == "MeshRender") {
            auto meshRender = dynamic_cast<MeshRender *>(component);
            meshRender->build();
        }
    }

    auto targetGameObject = entity->getGameObject();
    if (!targetGameObject) {
        throw std::runtime_error("GameObject is invalid");
    }

    auto &transform = targetGameObject->getTransform();

    auto &localTransform = m_gameObject->getTransform();

    localTransform.setLocalScale(transform.getLocalScale());
    localTransform.setLocalRotation(transform.getLocalRotation());
    localTransform.setLocalPosition(transform.getLocalPosition());

    if (auto parent = transform.getParent()) {
        localTransform.setParent(parent);
    } else {
        Scene::GetCurrentScene()->Add(getGameObject());
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

Component *Entity::addInternalComponent(const std::string &name) {
    if (!ScriptManager::CreateComponentWithName) {
        throw std::runtime_error("Failed to process Component: CreateComponentWithName is null");
    }

    auto component = ScriptManager::CreateComponentWithName(name);
    if (!component) {
        throw std::runtime_error("Failed to generate component for name: " + name);
    }
    
    component->entity = this;
    component->Init();

    m_componentPtr.emplace(name, std::shared_ptr<Component>(component));

    m_components.push_back(component);

    return component;
}

