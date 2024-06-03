#pragma once

class Entity;

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

    /* Quando o script terminar */
    virtual ~Component() {}
};
