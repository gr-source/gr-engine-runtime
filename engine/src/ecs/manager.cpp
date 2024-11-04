#include "manager.hpp"

#include "gameObject.h"
#include "settings.h"

#include "script/transform.hpp"

#include "core/file/stream.hpp"
#include "core/file/file.hpp"

#include "core/console.hpp"

std::unordered_map<EntityID, std::unique_ptr<Entity>> Manager::m_mapEntities;

std::vector<EntityID> Manager::m_available;

std::vector<Entity *> Manager::m_entities;

grm::size Manager::m_entityCount = 0;

bool Manager::bPlayScript = true;

std::mutex Manager::m_mutex;

Entity *Manager::CreateEntity(GameObject * gameObject) {
    if (!gameObject) {
        throw std::runtime_error("GameObject ptr is not valid.");
    }

    std::lock_guard<std::mutex> lock(m_mutex);
    auto hasAvailable = !m_available.empty();

    auto id = (hasAvailable ? m_available.back() : m_entityCount++);
    if (hasAvailable) {
        m_available.pop_back();
    }
    
    auto &entity = m_mapEntities[id] = std::make_unique<Entity>(gameObject, id);

    return m_entities.emplace_back(&(*entity));
}

void Manager::DestroyEntity(Entity *entity) {
    if (!entity) {
        throw std::runtime_error("Entity is not valid.");
    }

    auto id = entity->getEntityID();

    std::lock_guard<std::mutex> lock(m_mutex);
    m_available.push_back(id);

    m_mapEntities.erase(id);

    m_entities.erase(std::remove(m_entities.begin(), m_entities.end(), entity), m_entities.end());
}

void Manager::Update() {
    if (!Manager::bPlayScript) {
        return;
    }
    
    auto pages = (m_entities.size() + MAX_PAGES - 1) / MAX_PAGES;
    for (auto i=0;i<pages;i++) {
        auto start = i * MAX_PAGES;
        auto end = std::min(start + MAX_PAGES, static_cast<int>(m_entities.size()));

        for (auto j=start;j<end;j++) {
            if (!m_entities[j]->isActive()) {
                continue;
            }

            m_entities[j]->update();
        }
    }
}

grm::size Manager::GetEntityCount() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_mapEntities.size();
}

void Manager::Clear() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_mapEntities.clear();

    m_available.clear();

    m_entities.clear();

    m_entityCount = 0;
}

bool Manager::IsAvailable() {
    return bPlayScript;
}

bool Manager::IsValid(EntityID id) {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_mapEntities.find(id) != m_mapEntities.end();
}

Entity *Manager::GetEntity(EntityID id) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_mapEntities.find(id);
    if (it == m_mapEntities.end()) {
        return nullptr;
    }
    return &(*it->second);
}
