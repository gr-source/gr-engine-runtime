#include "Scene.h"
    
Scene::Scene(const std::string& name) : name(name){}

void Scene::remove(const GameObject* gameObject) {
    m_gameObjects.erase(std::remove(m_gameObjects.begin(), m_gameObjects.end(), gameObject), m_gameObjects.end());
}


