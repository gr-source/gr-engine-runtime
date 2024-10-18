#include "manager.hpp"

#include "gameObject.h"
#include "console.hpp"
#include "settings.h"

#include "script/transform.hpp"

#include "core/file/streaming.hpp"
#include "core/file/stream.hpp"
#include "core/file/file.hpp"

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
            if (!m_entity[j]->isActive()) {
                continue;
            }

            m_entity[j]->update();
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

std::list<std::tuple<Entity *, std::string>> m_vali;

void Manager::Replay() {
    // if (Manager::bPlayScript) {
    //     return;
    // }

    // for (auto &&[entity, name] : m_vali) {
    //     Component *component = nullptr;

    //     try {
    //         component = entity->hasComponent(name) ? entity->getComponent(name) : entity->addInternalComponent(name);
    //     } catch (const std::runtime_error &e) {
    //         Console::log(LogInfo::ERROR, "Fatal error: " + std::string(e.what()));
    //         continue;
    //     }

    //     if (name == "Transform") {
    //         entity->getGameObject()->setTransform(dynamic_cast<Transform *>(component));
    //     }

    //     Streaming stream(File::GetCacheFile("/" + name + "_" + std::to_string(entity->getEntityID())), std::ios_base::in | std::ios_base::binary);
    //     if (!stream.is_open()) {
    //         LOG_ERROR("[ Manager ] Falied to read component data: " + name);
    //         continue;
    //     }
    //     component->Deserialize(stream);
    //     stream.close();
    // }

    // m_vali.clear();
    // Manager::bPlayScript = true;
}

void Manager::Stop() {
    // for (auto &entity : m_entity) {
    //     for (auto &&[name, component] : entity->getComponents()) {
    //         m_vali.emplace_front(entity, name);

    //         Streaming stream(File::GetCacheFile("/" + name + "_" + std::to_string(entity->getEntityID())), std::ios_base::out | std::ios_base::binary);
    //         component->Serialize(stream);
    //         stream.close();
    //     }

    //     // entity->clear();
    // }
    // Manager::bPlayScript = false;
}

bool Manager::IsAvailable() {
    return bPlayScript;
}

bool Manager::IsValid(EntityID id) {
    return m_mapEntities.find(id) != m_mapEntities.end();
}

Entity *Manager::GetEntity(EntityID id) {
    auto it = m_mapEntities.find(id);
    if (it == m_mapEntities.end()) {
        return nullptr;
    }
    return &(*it->second);
}
