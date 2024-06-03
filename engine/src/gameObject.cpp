#include "gameObject.h"

#include "component/Transform.h"

#include "ecs/entityManager.h"

GameObject::GameObject(const std::string& name, const std::string& tag) : name(name), tag(tag), isStatic(false), isDelete(false), entity(&EntityManager::CreateEntity()) {
    entity->addComponent<Transform>();
}

GameObject::~GameObject() {
    EntityManager::remove(*entity);
}

const std::string &GameObject::getname() const {
    return name;
}

const std::string &GameObject::gettag() const {
    return tag;
}

Transform& GameObject::gettransform() {
    return entity->getComponent<Transform>();
}

bool GameObject::operator ==(const GameObject& other) const {
    return this == &other;
}
