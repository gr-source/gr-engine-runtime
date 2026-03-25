#pragma once

#include "default.hpp"

#include "viewlayout.hpp"

#include <gr-base/core/default.hpp>

#include <gr-render/gCommon.h>

#include "texteditor/textEditor.h"

#include <memory>
#include <vector>

class ShaderEditor :  public ViewLayout
{
public:
    ShaderEditor();
    ~ShaderEditor() override;

    void onFocus() override;

    void onBlur() override;

    void onRenderUI() override;

private:
    std::vector<std::tuple<std::string, gr::Shader *, std::unique_ptr<TextEditor>>> v_shader_texeditor;

    ImVec2 m_last_view_size;

    Scene *m_currentscene;

    Engine *e_engine;

    void create_window_viewport();

    void create_render_viewport();

    void create_window_texeditor();

    void create_render_texteditor();

    void create_menubar();

    void create_view_render();
};



