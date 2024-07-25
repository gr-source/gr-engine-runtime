#pragma once

#include "gameObject.h"

class ObjectManager {
public:
    static GameObject* find(const std::string& name);
    
    // template <typename... Args>
    // static GameObject& add(Args&&... args);
    
    static std::vector<std::unique_ptr<GameObject>>& getobjects();

    static const u32 getcount();

private:
    static std::vector<std::unique_ptr<GameObject>> m_gameobjects;
};

/*
template <typename... Args>
inline GameObject &ObjectManager::add(Args &&...args) {
    return *m_gameobjects.emplace_back(std::make_unique<GameObject>(std::forward<Args>(args)...));
}
*/



