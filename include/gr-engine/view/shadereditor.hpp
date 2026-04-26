#pragma once

// #include "default.hpp"

#include "gr-engine/ui/viewlayout.hpp"

#include <gr-base/core/default.hpp>

#include <gr-render/gCommon.h>

class ShaderEditor :  public ViewLayout
{
public:
    ShaderEditor();
    ~ShaderEditor() override;

    inline const char* getName() const override
    {
        return "Shader Editor";
    }

    void onFocus() override;

    void onBlur() override;

    void onRenderUI() override;
};



