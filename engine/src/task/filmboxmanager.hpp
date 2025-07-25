#pragma once

#include "filmboxtask.hpp"

#include <atomic>
#include <unordered_set>
#include <vector>

class FilmboxManager
{
public:
    template <typename ...Args>
    static std::shared_ptr<TaskInfo> Load(const std::string &path, Args &&...args);

    static void Render();

    void create_save(const std::string &name, GameObject * root);

private:
    static FilmboxManager &GetInstance();

    std::vector<GameObject *> resultTask;

    std::unordered_set<std::string> taskResultMap;
    
    std::mutex mutex;

    FilmboxManager();
    ~FilmboxManager();

    std::shared_ptr<TaskInfo> load(const std::string &path, std::function<void ()> func);

    void render();

    void render_tree_node(const GameObject *gameObject);

    void GameObjectSave(GameObject *go);
};

template <typename ...Args>
inline std::shared_ptr<TaskInfo> FilmboxManager::Load(const std::string &path, Args &&...args)
{
    std::function<void (void)> func = nullptr;
    if constexpr (sizeof...(Args) > 0)
    {
        func = std::bind(std::forward<Args>(args)...);
    }

    return GetInstance().load(path, func);
}






