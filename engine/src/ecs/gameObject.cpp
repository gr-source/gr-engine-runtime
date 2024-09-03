#include "gameObject.h"

#include "entity.h"

#include "../scriptManager/scriptManager.hpp"

#include "script/transform.hpp"

#include "core/scene.hpp"
#include "core/event.hpp"

#include "manager.hpp"
#include "gameObject.h"

std::unordered_map<std::string, std::unique_ptr<GameObject>> GameObject::s_mapObject;

std::unordered_map<std::string, int> GameObject::s_names;

GameObject &GameObject::Create(const std::string &name, const std::string &tag) {
    auto newName = AddNameID(name);

    auto &&[it, added] = s_mapObject.emplace(newName, std::make_unique<GameObject>(newName, tag));

    auto &gameObject = it->second;

    gameObject->m_transform = &gameObject->addComponent<Transform>();

    return *gameObject;
}

GameObject &GameObject::Instantiate(GameObject *target) {
    auto &gameObject = GameObject::Create(target->m_name, target->m_tag);

    gameObject.getEntity()->copy(target->getEntity());

    return gameObject;
}

GameObject *GameObject::Find(const std::string &name) {
    auto it = s_mapObject.find(name);
    if (it == s_mapObject.end()) {
        return nullptr;
    }
    return &(*it->second);
}

void GameObject::Rename(GameObject *gameObject, std::string name) {
    auto it = s_mapObject.find(gameObject->getName());
    if (it == s_mapObject.end()) {
        return;
    }

    name = AddNameID(String::RemoveNumber(name));

    auto ptr = std::move(it->second);

    s_mapObject.erase(it);

    s_mapObject.emplace(name, std::move(ptr));
    
    gameObject->setName(name);
}

void GameObject::Destroy(GameObject *target) {
    if (!target) {
        return;
    }

    auto &transform = target->getTransform();
    transform.setParent(nullptr);

    for (auto &child : transform.getChildren()) {
        auto gameObject = child->getGameObject();

        Event::Add(&GameObject::Destroy, gameObject);
    }

    auto it = s_mapObject.find(target->getName());
    if (it == s_mapObject.end()) {
        return;
    }

    s_mapObject.erase(it);
}

grm::size GameObject::GetGameObjectCount() {
    return s_mapObject.size();
}

void GameObject::Clear() {
    s_mapObject.clear();
    s_names.clear();
}

GameObject::GameObject(const std::string &name, const std::string &tag) : m_name(name), m_tag(tag), m_entity(&Manager::CreateEntity(this)), m_bActive(true), m_bEnable(true) {}

GameObject::~GameObject() {
    Manager::DestroyEntity(m_entity);
}

const std::string &GameObject::getName() {
    return m_name;
}

void GameObject::setName(const std::string &name) {
    m_name = name;
}

const std::string &GameObject::getTag() {
    return m_tag;
}

void GameObject::setActive(bool value) {
    m_bActive = value;
}

bool GameObject::isActive() const {
    return m_bActive && m_bEnable;
}

Transform &GameObject::getTransform() {
    return *m_transform;
}

Entity *GameObject::getEntity() {
    return m_entity;
}

void GameObject::setEnable(bool value) {
    assert(Entity::SetEnable && "Entity set enable point invalid");
    Entity::SetEnable(m_entity, value);
}   

std::string GameObject::AddNameID(const std::string &name) {
    auto it = s_names.find(name);
    if (it == s_names.end()) {
        s_names.emplace(name, 0);
    }
    return name + std::to_string(s_names[name]++);
}

void GameObject::RemoveNameID(const std::string &name) {
    auto it = s_names.find(name);
    if (it == s_names.end()) {
        return;
    }
    s_names[name]--;
}
