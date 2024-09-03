#include "manager.hpp"

#include "gameObject.h"

#include "settings.h"

#include <algorithm>

std::unordered_map<EntityID, std::unique_ptr<Entity>> Manager::m_mapEntities;

std::vector<EntityID> Manager::m_available;

std::vector<Entity *> Manager::m_entity;

grm::size Manager::m_entityCount = 0;

bool Manager::bPlayScript = true;

Entity &Manager::CreateEntity(GameObject * gameObject) {
    if (!gameObject) {
        throw std::runtime_error("GameObject ptr is not valid.");
    }
    auto hasAvailable = !m_available.empty();

    auto id = (hasAvailable ? m_available.back() : m_entityCount++);
    if (hasAvailable) {
        m_available.pop_back();
    }
    
    auto &entity = m_mapEntities[id] = std::make_unique<Entity>(gameObject, id);

    return *m_entity.emplace_back(&(*entity));
}

void Manager::DestroyEntity(Entity *entity) {
    if (!entity) {
        throw std::runtime_error("Entity is not valid.");
    }

    auto id = entity->getEntityID();

    m_available.push_back(id);

    m_mapEntities[id] = nullptr;

    m_entity.erase(std::remove(m_entity.begin(), m_entity.end(), entity));
}

void Manager::Update() {
    if (!Manager::bPlayScript) {
        return;
    }
    auto pages = (m_entity.size() + MAX_PAGES - 1) / MAX_PAGES;
    for (auto i=0;i<pages;i++) {
        auto start = i * MAX_PAGES;
        auto end = std::min(start + MAX_PAGES, static_cast<int>(m_entity.size()));

        for (auto j=start;j<end;j++) {
            if (!m_entity[j]->isActive() || !Entity::Update) {
                continue;
            }

            Entity::Update(m_entity[j]);
        }
    }
}

grm::size Manager::GetEntityCount() {
    return m_mapEntities.size();
}

void Manager::Clear() {
    m_mapEntities.clear();

    m_available.clear();

    m_entity.clear();

    m_entityCount = 0;
}

void Manager::Replay() {
    auto pages = (m_entity.size() + MAX_PAGES - 1) / MAX_PAGES;
    for (auto i=0;i<pages;i++) {
        auto start = i * MAX_PAGES;
        auto end = std::min(start + MAX_PAGES, static_cast<int>(m_entity.size()));

        for (auto j=start;j<end;j++) {
            if (!m_entity[j]->isActive() || !Entity::Update) {
                continue;
            }

            Entity::Init(m_entity[j]);
        }
    }
}

void Manager::Stop() {
    Manager::bPlayScript = true;
}
