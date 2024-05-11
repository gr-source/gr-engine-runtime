#include "Entity.h"

#include "../settings.h"
#include "../Global.h"

Entity::Entity() : IsActivate(false) {
    for (std::size_t i={};i<maxComponent;++i) {
        components[i] = nullptr;
    }
}

void Entity::OnStart() {
    std::for_each(components.begin(), components.end(), [&](const std::shared_ptr<Component>& current) {
        if (current == nullptr) return;
        #if IS_GAME
        current->OnStart();
        #else
        if (current->active || Settings::IsGameRunning || IsActivate)
            current->OnStart();
        #endif
    });
}

void Entity::OnUpdate() {
    /* pega lista de ponteiros reais e itere todos atualizando o update */
    std::for_each(components.begin(), components.end(), [&](const std::shared_ptr<Component>& current) {
        if (current == nullptr) return;
        #if IS_GAME
        if (Settings::IsGameRunning)
        current->OnUpdate();
        #else
        if (current->active || Settings::IsGameRunning)
            current->OnUpdate();
        #endif
    });
}

void Entity::OnRender(DebugDraw& debugDraw) {
    std::for_each(components.begin(), components.end(), [&](const std::shared_ptr<Component>& current) {
        if (current == nullptr) return;
        #if IS_GAME
        if (Settings::IsGameRunning)
        current->OnRender(debugDraw);
        #else
        if (current->active || Settings::IsGameRunning)
            current->OnRender(debugDraw);
        #endif
    });
}

void Entity::OnGui() {
    std::for_each(components.begin(), components.end(), [](std::shared_ptr<Component>& current) {
        if (current == nullptr) return;
        #if !IS_GAME
            current->OnGui();
        #endif
    });
}

void Entity::OnCollidedWith(const GameObject& other) {
    std::for_each(components.begin(), components.end(), [&](std::shared_ptr<Component>& it) {
        if (it == nullptr) return;
        it->OnCollidedWith(other);
    });
}

bool Entity::operator==(const Entity& other) const {
    return this == &other;
}







