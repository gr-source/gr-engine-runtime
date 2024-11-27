#pragma once

#include "default.hpp"

#include <gr-math/gmath.hpp>

#include <unordered_map>
#include <vector>
#include <memory>
#include <mutex>

class Manager {
public:
    Manager() = delete;
    
    static Entity *CreateEntity(GameObject *gameObject);

    static void DestroyEntity(Entity *entity);

    static void Update();

    static grm::size GetEntityCount();

    static void Clear();

private:
    static std::unordered_map<EntityID, std::unique_ptr<Entity>> m_mapEntities;

    static std::vector<EntityID> m_available;

    static std::vector<Entity *> m_entities;

    static grm::size m_entityCount;

    static std::mutex m_mutex;

    static bool IsValid(EntityID id);

    static Entity *GetEntity(EntityID id);
};


