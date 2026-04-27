#include <cmath>
#include <cstdlib>
#include <gr-math/types.hpp>
#include <iostream>

#include <gr-base/core/mainthreaddispatcher.hpp>
#include <gr-base/core/gameObject.hpp>
#include <gr-base/core/clock.hpp>

#include <gr-base/ecs/behavior.hpp>

#include <gr-base/script/transform.hpp>

class ScriptManager
{
public:
    static ScriptManager& GetInstance()
    {
        static ScriptManager s_ScriptManager;
        return s_ScriptManager;
    }

    bool Add(uint64_t hash)
    {
        m_scripts.push_back(hash);
        return true;
    }

    std::vector<uint64_t> GetScripts() const
    {
        return m_scripts;
    }

private:
    std::vector<uint64_t> m_scripts;
};

#define REGISTER_BEHAVIOR(k) \
    REGISTER_COMPONENT(k) \
    inline static bool k##_Registry = ScriptManager::GetInstance().Add(k##Hash);

class GameController : public BehaviorImpl<GameController>
{
public:
    GameController() {}

    void Start() override
    {
        std::cout << "GameController::Start" << std::endl;
    }

    void Update() override
    {
    }
};

REGISTER_BEHAVIOR(GameController)

__attribute__((constructor))
static void Initialize()
{
    std::cout << "[ Script ] Initialize..." << std::endl;
}

__attribute__((destructor))
static void Shutdown()
{
    std::cout << "[ Script ] Shutdown..." << std::endl;

    // for (uint64_t hash : ScriptManager::GetInstance().GetScripts())
        // ComponentManager::Unregistry(hash);
}

