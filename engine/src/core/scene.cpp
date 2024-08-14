#include "scene.hpp"

#include "../scriptManager/scriptManager.hpp"

std::list<GameObject *> &(*Scene::GetGameObject)() = nullptr;

void (*Scene::Delete)(GameObject *target);

void Scene::Remove(GameObject *target) {
    if (!Scene::GetGameObject) {
        return;
    }
    auto &list = Scene::GetGameObject();
    list.erase(std::remove(list.begin(), list.end(), target), list.end());
}

void Scene::Add(GameObject *target) {
    if (!Scene::GetGameObject) {
        return;
    }
    auto &list = Scene::GetGameObject();
    auto it = std::find(list.begin(), list.end(), target);
    if (it != list.end()) {
        return;
    }
    list.push_front(target);
}
