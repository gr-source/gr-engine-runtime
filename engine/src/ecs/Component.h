#pragma once

#include "../renderer.h"

class Component {
public:
    Component(bool active = false) : active(active), entity(nullptr) {}

    /* Entidade atual */
    Entity* entity;

    /* Scipt esta ativa */
    bool active;

    /* Quando a classe for iniciada */
    virtual void OnInit() {}

    /* Quando o script e chamado */
    virtual void OnStart() {}

    /* Atualização do script */
    virtual void OnUpdate() {}

    /* Renderização do script */
    virtual void OnRender(DebugDraw& debugDraw) {}

    /* o objeto que a entidade atual collidiu */
    virtual void OnCollidedWith(const GameObject& other) {}

    /* UI de propriedades do script (ENGINE) */
    virtual void OnGui() {}

    /* Quando o script terminar */
    virtual ~Component() {}
};
