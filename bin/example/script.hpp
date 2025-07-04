#pragma once

#include <gr-base/script/default.hpp>

#include <gr-base/ecs/entity.hpp>

class #name : public Component
{
    CLASS_FUNCTIONS
public:
    #name();

    void Init() override;

    void Update() override;

    void Destroy() override;
};

REGISTER_COMPONENT(#name, true)

