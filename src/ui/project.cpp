#include "gr-engine/ui/project.hpp"
#include <cstddef>

extern "C"
{
#include <gr-ini/config.h>
}

#include <gr-base/core/file/file.hpp>

void Project::save()
{
    auto config = config_create();
    if (config == nullptr)
        return;

    {
        auto section = config_create_section(config, "project");
        config_set_string(section, "name", name.c_str());
        config_set_string(section, "path", path.c_str());
        config_set_string(section, "version", version.c_str());
    }

    config_save(config, (gfile::get_config_directory() + "/config.ini").c_str());

    config_release(config);
}

bool Project::load(const std::string& file)
{
    gConfig* config = config_create();
    if (config == NULL)
        return false;

    bool success = true;
    if (!config_load(config, file.c_str()))
    {
        success = false;
    } else
    {
        gSection* section = config_find_section(config, "project");
        if (section == NULL)
        {
            success = false;
        } else
        {
            const char* c_name = config_get_string(section, "name");
            const char* c_path = config_get_string(section, "path");
            const char* c_version = config_get_string(section, "version");

            if (c_name == NULL || c_path == NULL || c_version == NULL || !gfile::exists(c_path))
            {
                success = false;
            } else
            {
                name = c_name;
                path = c_path;
                version = c_version;
            }
        }
    }

    config_release(config);
    return success;
}


