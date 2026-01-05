#include "shadereditor.hpp"

#include "editorController.h"
#include "engine.h"

#include <fstream>

#include <gr-base/managers/assetsManager.hpp>

#include <gr-base/core/scene.hpp>

#include <gr-base/core/file/file.hpp>

#include <gr-base/script/skinnedMeshRender.hpp>
#include <gr-base/script/meshRender.hpp>
#include <gr-base/script/camera.hpp>

#include <gr-base/ecs/gameObject.hpp>

#include "imgui.h"

ShaderEditor::ShaderEditor() : e_engine(&Engine::GetInstance()), m_currentscene(nullptr), m_last_view_size(0.0f, 0.0f)
{
}

ShaderEditor::~ShaderEditor()
{}

void ShaderEditor::onFocus()
{
    // m_currentscene = Scene::GetCurrentScene();

    /*
    auto objects = m_currentscene->get_object_selected();
    if (!objects.empty())
    {
        auto go = objects.back();

        BaseRender *baserender = nullptr;
        if (go->hasComponent<MeshRender>())
        {
            baserender = static_cast<BaseRender *>(go->getComponent<MeshRender>());
        } else if (go->hasComponent<SkinnedMeshRender>())
        {
            baserender = static_cast<BaseRender *>(go->getComponent<SkinnedMeshRender>());
        }

        if (baserender != nullptr)
        {
            for (auto &materialID : baserender->getMaterial())
            {
                auto &material = AssetsManager::GetMaterial(materialID);

                auto shader = material.getShader();
                if (shader == nullptr)
                {
                    continue;
                }

                auto name = AssetsManager::GetShaderName(shader);
                auto path = name;
                
                auto end = name.find(';');
                if (end != std::string::npos)
                {
                    path = name.substr(0, end);
                }

                auto &&[p, s, texteditor] = v_shader_texeditor.emplace_back(name, shader, std::make_unique<TextEditor>());

                auto content = File::GetFileContent(path);

                texteditor->SetText(content);
            }
        }
    }
    */

    EditorController::bCanMove = false;
}

void ShaderEditor::onBlur()
{
    v_shader_texeditor.clear();
    
    EditorController::bCanMove = true;
}

void ShaderEditor::onRenderUI()
{
    if (m_currentscene == nullptr)
    {
        return;
    }
    create_window_viewport();

    create_window_texeditor();
}

// ********** private ********** //
void ShaderEditor::create_window_viewport()
{
    if (ImGui::Begin("viewport - se"))
    {
        create_render_viewport();
    }
    ImGui::End();
}

void ShaderEditor::create_render_viewport()
{
    auto camera = m_currentscene->getCamera();

    auto &rendertarget = camera->rendertarget;

    auto avail = ImGui::GetContentRegionAvail();
    if (m_last_view_size.x != avail.x || m_last_view_size.y != avail.y)
    {
        camera->setAspectRatio(avail.x / avail.y);

        m_last_view_size = avail;
    }

    ImGui::Image(reinterpret_cast<ImTextureID>(rendertarget.getTextureID()), avail, {0.0f, 1.0f}, {1.0f, 0.0f});
}

void ShaderEditor::create_window_texeditor()
{
    if (ImGui::Begin("texeditor", nullptr, ImGuiWindowFlags_MenuBar))
    {
        create_render_texteditor();
    }
    create_menubar();
    
    ImGui::End();
}

void ShaderEditor::create_render_texteditor()
{
    for (auto &&[path, shader, texteditor] : v_shader_texeditor)
        texteditor->Render(path.c_str());
}

void ShaderEditor::create_menubar()
{
    if (!ImGui::BeginMenuBar())
    {
        return;
    }

    auto &io = ImGui::GetIO();
    if (io.KeyCtrl && ImGui::IsKeyReleased(ImGuiKey_S))
    {
        for (auto &&[path, shader, texteditor] : v_shader_texeditor)
        {
            {
                std::ofstream os(path);
                os << texteditor->GetText() << std::endl;
                os.close();
            }

            throw std::runtime_error("Falta arrrumar aqui: 55 - ShaderEditor");

            // AssetsManager::LoadShader(path, shader);
        }
    }
    ImGui::EndMenuBar();
}

void ShaderEditor::create_view_render()
{
}




