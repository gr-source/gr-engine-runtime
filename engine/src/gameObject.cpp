#include "gameObject.h"

#include "component/Transform.h"
#include "component/camera.h"

#include "ecs/entityManager.h"


GameObject::GameObject(const std::string& name, const std::string& tag) : m_name(name), m_tag(tag), m_entity(&EntityManager::CreateEntity(this)), m_active(true) {
    m_entity->addComponent<Transform>();
}

GameObject::~GameObject() {
    EntityManager::remove(*m_entity);
}

const std::string &GameObject::getName() const {
    return m_name;
}

const std::string &GameObject::getTag() const {
    return m_tag;
}

void GameObject::setActive(bool active) {
    if (hasComponent<Camera>() && active) {
        Camera::s_current = &getComponent<Camera>();
    }
    m_active = active;
}

const bool &GameObject::getActive() const {
    return m_active;
}

Transform& GameObject::getTransform() {
    return m_entity->getComponent<Transform>();
}

bool GameObject::operator ==(const GameObject& other) const {
    return this == &other;
}
