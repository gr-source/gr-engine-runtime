#include "entityManager.h"
#include "entity.h"

#include "../settings.h"

std::vector<std::unique_ptr<Entity>> EntityManager::m_entities;

Entity& EntityManager::CreateEntity() {
    return *m_entities.emplace_back(std::make_unique<Entity>());
}

void EntityManager::remove(const Entity& entitie) {
    m_entities.erase(std::remove_if(m_entities.begin(), m_entities.end(),
        [&entitie](const std::unique_ptr<Entity>& e) { return e.get() == &entitie; }), 
        m_entities.end());
}

void EntityManager::OnUpdate() { 
    /* percorra todas as entidades e o os atualize */
    std::for_each(m_entities.begin(), m_entities.end(), [](std::unique_ptr<Entity>& entitie) {
        entitie->OnUpdate();
    });
}

void EntityManager::OnPlayGame() {
    /* perocrra todas as entidades e abilites os scripts */
    std::for_each(m_entities.begin(), m_entities.end(), [](std::unique_ptr<Entity>& entitie) {
        entitie->IsActivate = true;
        entitie->OnStart();
    });
}

void EntityManager::OnStopPlayGame() {
    /* perocrra todas as entidades e desabilite os scripts */
    std::for_each(m_entities.begin(), m_entities.end(), [](std::unique_ptr<Entity>& entitie) {
        entitie->IsActivate = false;
    });
}

const u32 EntityManager::getcount() {
    return m_entities.size();
}
