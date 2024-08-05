#include "scene.hpp"

std::list<GameObject *> &(*Scene::GetGameObject)() = nullptr;

void Scene::Remove(GameObject *target) {
    if (!Scene::GetGameObject) {
        return;
    }
    auto &list = Scene::GetGameObject();
    list.remove(target);
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
    list.push_back(target);
}
