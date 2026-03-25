#include "gr-engine/view/installer.hpp"

#include "gr-engine/engine.h"

#include "gr-base/core/mainthreaddispatcher.hpp"

#include "imgui.h"

#include "gr-engine/managers/viewmanager.hpp"

#include <gr-base/core/thread/taskmanager.hpp>
#include <gr-base/core/thread/threadpool.hpp>
#include <gr-base/core/console.hpp>

#include <gr-base/core/file/file.hpp>

Installer::Installer() : bFinnish(false)
{
    Threadpool::AddTask(&Installer::doInBackground, this);
}

const char* Installer::getName() const
{
    return "Installer";
}

void Installer::onRenderUI()
{
    auto mainViewport = ImGui::GetMainViewport();

    ImVec2 window_size = {600.0f, 500.0f};

    ImGui::SetNextWindowSize(window_size, ImGuiCond_Always);
    ImGui::SetNextWindowPos((mainViewport->WorkSize - window_size) / 2.0f);
    ImGui::Begin("Installer", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

    auto avail = ImGui::GetContentRegionAvail();

    for (auto &log : Console::GetLogs())
    {
        ImVec4 color = {1.0f, 1.0f, 1.0f, 1.0f};
        std::string type = "[ DEFAULT ]";

        switch (log.type)
        {
        case Log::LogType::INFO:
            color = {0.0f, 1.0f, 0.0f, 1.0f};
            type = "[ INFO ]";
            break;
        case Log::LogType::WARNING:
            color = {1.0f, 1.0f, 0.0f, 1.0f};
            type = "[ WARNING ]";
            break;
        case Log::LogType::ERROR:
            color = {1.0f, 0.0f, 0.0f, 1.0f};
            type = "[ ERROR ]";
            break;
        default:
            break;
        }

        ImGui::TextColored(color, "%s", type.c_str());
        ImGui::SameLine();
        ImGui::Text("%s", log.message.c_str());
    }

    if (Console::IsChanged())
    {
        ImGui::SetScrollHereY(1.0f);
        Console::StopChange();
    }

    if (bFinnish && ImGui::Button("Finish", ImVec2(avail.x, 0.0f)))
    {
        bFinnish = false;

        MainThreadDispatcher::Add(Priority::NORMAL, &Engine::init_menu, Engine::GetInstance());
    }
    ImGui::End();
}

// private
void Installer::doInBackground()
{
    Console::LogInfo("[ Installer ] iniciando instalação...");

    if (!gfile::exists(gfile::get_home_directory() + "/gr-engine"))
    {
        gfile::create_directory(gfile::get_home_directory() + "/gr-engine");
    }

    if (!gfile::exists(gfile::get_data_directory()))
    {
        gfile::create_directory(gfile::get_data_directory());
    }

    if (!gfile::exists(gfile::get_config_directory()))
    {
        gfile::create_directory(gfile::get_config_directory());
    }

    if (!gfile::exists(gfile::get_cache_directory()))
    {
        gfile::create_directory(gfile::get_cache_directory());
    }

    Console::LogInfo("[ Installer ] copiando arquivos...");

    if (!download_data())
    {
        gfile::remove(gfile::get_data_directory());
        gfile::remove(gfile::get_config_directory());
        gfile::remove(gfile::get_cache_directory());
        return;
    }

    Console::LogInfo("[ Installer ] arquivos copiados com sucesso.");

    std::this_thread::sleep_for(std::chrono::seconds(3));

    bFinnish = true;
}

bool Installer::download_data()
{
    Console::LogInfo("[ Installer ] copiando assets...");
    if (!gfile::copy("./bin/assets", (gfile::get_data_directory() + "/assets").c_str()))
    {
        Console::LogError("[ Installer ] fail to copy assets.");
        return false;
    }

    Console::LogInfo("[ Installer ] copiando scripts...");
    if (!gfile::copy("./bin/script", (gfile::get_data_directory() + "/script").c_str()))
    {
        Console::LogError("[ Installer ] fail to copy script.");
        return false;
    }

    Console::LogInfo("[ Installer ] copiando arquivos de exemplo...");
    if (!gfile::copy("./bin/example", (gfile::get_data_directory() + "/example").c_str()))
    {
        Console::LogError("[ Installer ] fail to copy examples");
        return false;
    }

    return true;
}


