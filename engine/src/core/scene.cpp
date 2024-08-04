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
    list.push_back(target);
}
