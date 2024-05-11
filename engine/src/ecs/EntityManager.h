#pragma once

#include "Entity.h"

#include "../types.h"
#include "../renderer.h"

class EntityManager {
public:

    /* Crie um a entidade */
    static Entity& CreateEntity();

    /* remova uma entidade da lista (delete) */
    static void remove(const Entity& entitie);

    /* atualize as entidades */
    static void OnUpdate();

    /* Renderize as entidades */
    static void OnRender(DebugDraw& debugDraw);

    /* Inicie ao scripts das entidade */
    static void OnPlayGame();

    /* Pare os scripts das entidades */
    static void OnStopPlayGame();

    static const u32 getcount();

private:
    /* Lista de entidades(deleta automaticamente a entifade ao sair do scopo) */
    static std::vector<std::unique_ptr<Entity>> m_entities;
};












