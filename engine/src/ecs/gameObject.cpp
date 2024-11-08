#include "gameObject.h"

#include "entity.h"

#include "../scriptManager/scriptManager.hpp"

#include "script/meshRender.hpp"
#include "script/transform.hpp"

#include "core/nameManager.hpp"
#include "core/scene.hpp"
#include "core/event.hpp"
#include "core/scene.hpp"

#include "manager.hpp"
#include "gameObject.h"

std::unordered_map<std::string, std::unique_ptr<GameObject>> GameObject::m_dataObject;

GameObject *GameObject::Create(const std::string &name, const std::string &tag) {
    auto newName = NameManager::GenerateNameID(name);

    while (Find(newName)) {
        newName = NameManager::GenerateNameID(newName);
    }

    auto &&[it, added] = m_dataObject.emplace(newName, std::make_unique<GameObject>(newName, tag));
    if (!added) {
        throw std::runtime_error("GameObject has been replaced: " + newName);
    }

    auto &gameObject = it->second;

    gameObject->m_transform = gameObject->addComponent<Transform>();
    if (!gameObject->m_transform) {
        m_dataObject.erase(it);
        return nullptr;
    }

    return &(*gameObject);
}

GameObject *GameObject::Instantiate(GameObject *target) {
    auto gameObject = GameObject::Create(target->m_name, target->m_tag);
    if (!gameObject) {
        return nullptr;
    }
    
    gameObject->m_bEnable = target->m_bEnable;
    gameObject->m_bActive = target->m_bActive;

    gameObject->getEntity()->copy(target->getEntity());

    return gameObject;
}

GameObject *GameObject::Find(const std::string &name) {
    auto it = m_dataObject.find(name);
    if (it == m_dataObject.end()) {
        return nullptr;
    }
    return &(*it->second);
}

void GameObject::Rename(GameObject *gameObject, std::string name) {
    auto it = m_dataObject.find(gameObject->getName());
    if (it == m_dataObject.end()) {
        return;
    }

    name = NameManager::GenerateNameID(name);

    auto ptr = std::move(it->second);

    m_dataObject.erase(it);

    m_dataObject.emplace(name, std::move(ptr));
    
    gameObject->setName(name);
}

void GameObject::Destroy(GameObject *target) {
    auto sname = target->getName();

    NameManager::RemoveNameID(sname);

    auto it = m_dataObject.find(target->getName());
    if (it == m_dataObject.end()) {
        return;
    }

    auto transform = target->getTransform();
    // remove o objeto pai do objeto altual e se o pai me ter como filho me remove do objeto pai.
    transform->setParent(nullptr);

    auto& children = transform->getChildren();

    for (auto &child : children) {
        // como o objeto pai ja foi removido diz, aos filhos que não tem mais parent.
        child->setParent(nullptr, false);

        auto gameObject = child->getGameObject();

        GameObject::Destroy(gameObject);
    }

    m_dataObject.erase(it);
}

grm::size GameObject::GetGameObjectCount() {
    return m_dataObject.size();
}

void GameObject::Clear() {
    m_dataObject.clear();

    NameManager::Shutdown();
}

GameObject::GameObject(const std::string &name, const std::string &tag) : m_name(name), m_tag(tag), m_entity(Manager::CreateEntity(this)), m_bActive(true), m_bEnable(true) {}

GameObject::~GameObject() {
    Manager::DestroyEntity(m_entity);
}

void GameObject::initialize() {
    if (!m_entity || !m_transform) {
        return Console::LogError("[ GameObject ] Falied to initialize object: %s", m_name.c_str());
    }
    m_entity->init();

    if (hasComponent<MeshRender>()) {
        getComponent<MeshRender>()->build();
    }

    for (auto &child : m_transform->getChildren()) {
        child->getGameObject()->initialize();
    }
}

std::string GameObject::getName() const {
    return m_name;
}

void GameObject::setName(const std::string &name) {
    m_name = name;
}

std::string GameObject::getTag() const {
    return m_tag;
}

void GameObject::setActive(bool value) {
    m_bActive = value;

    for (auto child : m_transform->getChildren()) {
        child->getGameObject()->setActive(value);
    }
}

bool GameObject::isActive() const {
    return m_bActive && m_bEnable;
}

void GameObject::setTransform(Transform *transform) {
    m_transform = transform;
}

Transform *GameObject::getTransform() const {
    return m_transform;
}

Entity *GameObject::getEntity() const {
    return m_entity;
}

void GameObject::setEnable(bool value) {
    m_bEnable = value;
}

bool GameObject::getActive() const {
    return m_bActive;
}

bool GameObject::getEnable() const {
    return m_bEnable;
}








