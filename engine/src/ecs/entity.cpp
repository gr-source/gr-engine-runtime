#include "entity.h"

#include "../settings.h"

Entity::Entity(GameObject* gameobject) : m_gameobject(gameobject), IsActivate(false), components(32), m_bitset(0) {}

void Entity::OnStart() {
    std::for_each(components.begin(), components.end(), [&](auto& current) {
        #if IS_GAME
        current->OnStart();
        #else
        if (current.second->active || Settings::IsGameRunning || IsActivate)
            current.second->OnStart();
        #endif
    });
}

void Entity::OnUpdate() {
    /* pega lista de ponteiros reais e itere todos atualizando o update */
    std::for_each(components.begin(), components.end(), [&](auto& current) {
        #if IS_GAME
        if (Settings::IsGameRunning)
            current->OnUpdate();
        #else
        if (current.second->active || Settings::IsGameRunning)
            current.second->OnUpdate();
        #endif
    });
}

bool Entity::operator==(const Entity& other) const {
    return this == &other;
}







