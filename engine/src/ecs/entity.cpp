#include "entity.hpp"

#include "gameObject.hpp"

#include "../scriptManager/scriptManager.hpp"

#include "../settings.h"

#include "script/meshRender.hpp"
#include "script/transform.hpp"

#include "core/scene.hpp"

Entity::Entity(GameObject *gameObject, EntityID id) : m_gameObject(gameObject), m_entityID(id) {}

GameObject *Entity::getGameObject() const {
    return m_gameObject;
}

Component *Entity::addComponent(const std::string &name, const Component *instance) {
    return addInternalComponent(name, instance);
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

std::unordered_map<std::string, std::unique_ptr<Component>> &Entity::getComponents() {
    return m_componentPtr;
}

void Entity::copy(const Entity *entity) {
    for (const auto& [name, component] : entity->m_componentPtr) {
        if (hasComponent(name)) {
            continue;
        }

        addComponent(name, component.get());
    }

    auto targetGameObject = entity->getGameObject();
    if (!targetGameObject) {
        Console::LogError("[ Entity ] Failed to get game object pointer.");
        return;
    }

    auto targetTransform = targetGameObject->getTransform();
    auto localTransform = m_gameObject->getTransform();

    if (targetTransform) {
        localTransform->setLocalScale(targetTransform->getLocalScale());
        localTransform->setLocalRotation(targetTransform->getLocalRotation());
        localTransform->setLocalPosition(targetTransform->getLocalPosition());

        if (!targetTransform->hasParent()) {
            Scene::GetCurrentScene()->insert(getGameObject());
        } else {
            localTransform->setParent(targetTransform->getParent());
        }

        for (const auto& child : targetTransform->getChildren()) {
            if (!child) {
                Console::LogError("[ Entity ] Failed to copy child of %s, child pointer is invalid.", targetGameObject->getName().c_str());
                continue;
            }

            auto childGameObject = child->getGameObject();
            auto clonedObject = GameObject::Instantiate(childGameObject);
            if (!clonedObject) {
                Console::LogError("[ Entity ] Failed to create child object %s->%s.", targetGameObject->getName().c_str(), childGameObject->getName().c_str());
                continue;
            }

            auto clonedTransform = clonedObject->getTransform();
            if (clonedTransform) {
                clonedTransform->setParent(localTransform);
            }
        }
    } else {
        Console::LogError("[ Entity ] Target game object does not have a transform.");
    }

    init();

    if (hasComponent<MeshRender>()) {
        getComponent<MeshRender>()->build();
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

void Entity::update() {
    auto pages = (m_components.size() + MAX_PAGES - 1) / MAX_PAGES;
    for (auto i=0;i<pages;i++) {
        auto start = i * MAX_PAGES;
        auto end = std::min(start + MAX_PAGES, int(m_components.size()));

        for (auto j=start;j<end;j++) {
            m_components[j]->Update();
        }
    }
}

void Entity::init() {
    std::list<std::tuple<std::string, Component *>> temp;
    for (auto &&[name, component] : m_componentPtr) {
        temp.emplace_front(name, &(*component));
    }

    for (auto &&[name, component] : temp) {
        component->Init();
    }
} 

void Entity::clear() {
    m_components.clear();

    m_componentPtr.clear();
}

void Entity::remove(const std::string &name) {
    auto it = m_componentPtr.find(name);
    if (it == m_componentPtr.end()) {
        return;
    }

    auto component = &(*it->second);

    m_components.erase(std::remove(m_components.begin(), m_components.end(), component), m_components.end());
    m_componentPtr.erase(it);
}

Component *Entity::addInternalComponent(const std::string &name, const Component *instance) {
    if (!ScriptManager::CreateComponentWithName) {
        return nullptr;
    }

    auto component = instance == nullptr ? ScriptManager::CreateComponentWithName(name) : ScriptManager::CopyComponentWithName(name, instance);

    if (!component) {
        return nullptr;
    }
    
    component->entity = this;

    m_componentPtr.emplace(name, component);

    m_components.push_back(component);

    return component;
}
