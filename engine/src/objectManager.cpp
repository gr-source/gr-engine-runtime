#include "objectManager.h"

#include "gameObject.h"

std::vector<std::unique_ptr<GameObject>> ObjectManager::m_gameobjects;

GameObject* ObjectManager::find(const std::string &name) {
    auto it = std::find_if(m_gameobjects.begin(), m_gameobjects.end(), [&name](const std::unique_ptr<GameObject>& object) {
        return object->name == name;
    });
    if (it != m_gameobjects.end()) {
        return (*it).get();
    }
    return nullptr;
}

std::vector<std::unique_ptr<GameObject>>& ObjectManager::getobjects() {
    return m_gameobjects;
}

const u32 ObjectManager::getcount() {
    return m_gameobjects.size();
}

