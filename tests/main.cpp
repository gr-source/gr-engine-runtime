#include <gr-base/core/logger.hpp>

#include <gr-base/ecs/componentRegistry.hpp>
#include <gr-base/ecs/world.hpp>

#include <gr-base/script/default.hpp>

#include <iomanip>
#include <iostream>
#include <string>

#include <dirent.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include <sys/stat.h>
#include <chrono>

class ScopeTimer
{
    std::chrono::steady_clock::time_point m_start;

    std::string m_name;

public:
    ScopeTimer(const char* name) : m_name(name)
    {
        m_start = std::chrono::steady_clock::now();
    }

    ~ScopeTimer()
    {
        auto end =
                std::chrono::steady_clock::now();

            auto duration =
                std::chrono::duration<double, std::milli>(
                    end - m_start);

            std::cout
                << m_name
                << ": "
                << std::fixed
                << std::setprecision(3)
                << duration.count()
                << " ms\n";
    }
};

constexpr size_t EntityCount = 1000000;

struct Position
{
    float x, y, z;
};

template <>
inline constexpr TypeCategory engine::type_traits::category<Position> =
    TypeCategory::Component;

struct Velocity
{
    float x, y, z;
};

template <>
inline constexpr TypeCategory engine::type_traits::category<Velocity> =
    TypeCategory::Component;

struct Health
{
    int value;
};

template <>
inline constexpr TypeCategory engine::type_traits::category<Health> =
    TypeCategory::Component;

void runnable()
{
    engine::ecs::World world;

    std::vector<Entity> entities;
    entities.reserve(EntityCount);

    //--------------------------------------------------
    // Create
    //--------------------------------------------------
    {
        ScopeTimer timer("Create");

        for (size_t i = 0; i < EntityCount; ++i)
            entities.push_back(world.Create());
    }

    //--------------------------------------------------
    // Add Position
    //--------------------------------------------------
    {
        ScopeTimer timer("Add Position");

        for (auto e : entities)
            world.AddComponent<Position>(e);
    }

    //--------------------------------------------------
    // Add Velocity
    //--------------------------------------------------
    {
        ScopeTimer timer("Add Velocity");

        for (auto e : entities)
            world.AddComponent<Velocity>(e);
    }

    //--------------------------------------------------
    // Add Health
    //--------------------------------------------------
    {
        ScopeTimer timer("Add Health");

        for (auto e : entities)
            world.AddComponent<Health>(e);
    }

    //--------------------------------------------------
    // Get Component
    //--------------------------------------------------
    {
        ScopeTimer timer("Get Position");

        volatile float sum = 0;

        for (auto e : entities)
        {
            auto& p = world.GetComponent<Position>(e);
            sum += p.x;
        }
    }

    //--------------------------------------------------
    // Iterate Position
    //--------------------------------------------------
    {
        ScopeTimer timer("Iterate Position");

        world.Query<Position>([](
                    size_t count,
                    Entity* entities,
                    Position* positions)
        {
            for (size_t i=0;i<count;i++)
            {
                Position& p = positions[i];
                p.x += 1.0f;
            }
        });
    }

    //--------------------------------------------------
    // Iterate Position + Velocity
    //--------------------------------------------------
    {
        ScopeTimer timer("Iterate Position + Velocity");

        world.Query<Position, Velocity>(
            [](
                size_t count,
                Entity*,
                Position* positions,
                Velocity* velocities
            )
            {
                for (size_t i=0;i<count;i++)
                {
                    Position& p = positions[i];
                    Velocity& v = velocities[i];

                    p.x += v.x;
                    p.y += v.y;
                    p.z += v.z;
                }
            });
    }

    /*
    //--------------------------------------------------
    // Remove Component
    //--------------------------------------------------
    {
        ScopeTimer timer("Remove Velocity");

        for (auto e : entities)
            world.RemoveComponent<Velocity>(e);
    }
    */

    //--------------------------------------------------
    // Destroy
    //--------------------------------------------------
    {
        ScopeTimer timer("Destroy");

        for (auto e : entities)
            world.Destroy(e);
    }
}

int main()
{
    logger_config config;
    config.path = "./";

    logger::Initialize();

    ComponentRegistry::Initialize();
    RegistryDefaultComponent();

    for (int i=0;i<10;i++)
    {
        runnable();
    }

    ComponentRegistry::Shutdown();

    logger::Shutdown();

    /*
    char dir[512] = {0};
    if (getcwd(dir, sizeof(dir)) == NULL)
        return 1;

    char manifest_file[512] = {0};
    sprintf(manifest_file, "%s/bin/manifest.txt", dir);

    FILE* file = fopen(manifest_file, "w");

    char path[512];
    sprintf(path, "%s/bin/resources", dir);

    forearch_dir(path, NULL, file);

    fclose(file);
    */
}
