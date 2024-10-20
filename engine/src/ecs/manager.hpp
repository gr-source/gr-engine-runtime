#pragma once

#include "entity.h"

class Manager {
public:
    Manager() = delete;
    
    static Entity& CreateEntity(GameObject *gameObject);

    static void DestroyEntity(Entity *entity);

    static void Update();

    static grm::size GetEntityCount();

    static void Clear();

    static bool IsAvailable();

private:
    static std::unordered_map<EntityID, std::unique_ptr<Entity>> m_mapEntities;

    static std::vector<EntityID> m_available;

    static std::vector<Entity *> m_entity;

    static grm::size m_entityCount;

    static bool bPlayScript;

    static bool IsValid(EntityID id);

    static Entity *GetEntity(EntityID id);
};


