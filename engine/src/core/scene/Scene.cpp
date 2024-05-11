#include "Scene.h"

Scene::Scene(const std::string& name) : name(name){}

void Scene::add(GameObject *gameobject) {
    m_gameObjects.push_back(gameobject);
}

void Scene::remove(const GameObject* gameObject) {
    m_gameObjects.erase(std::remove(m_gameObjects.begin(), m_gameObjects.end(), gameObject), m_gameObjects.end());
}


