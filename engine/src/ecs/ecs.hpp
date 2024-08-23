#pragma once

#include "../scriptManager/scriptManager.hpp"

class Ecs {
public:
    template <typename T>
    static void for_each(std::function<void(GameObject *, T *)> func);
};

template <typename T>
inline void Ecs::for_each(std::function<void(GameObject *, T *)> func) {
    // assert(ScriptManager::GetAllGameObject && "Invalid.");

    // auto &list = ScriptManager::GetAllGameObject();

    // for (auto &gameObject : list) {
    //     if (!gameObject->hasComponent<T>()) {
    //         continue;
    //     }
    //     func(&(*gameObject), &gameObject->getComponent<T>());
    // }
}





