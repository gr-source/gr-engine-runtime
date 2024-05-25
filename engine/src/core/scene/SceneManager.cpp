#include "SceneManager.h"

#include <fstream>
#include <sstream>

Scene SceneManager::m_scene;

void SceneManager::Save(const std::string &path) {
    std::ofstream os(path);

    os << "name: " << m_scene.name << std::endl;

    for (const auto& object : m_scene.m_gameObjects) {
        os << "object " << object->name << std::endl;
    }

    os.close();
}

void SceneManager::Load(const std::string &path) {
    std::ifstream is(path);
    if (!is.is_open()) {
        throw std::runtime_error("Error file do notr open: " + path);
    }

    std::string line;
    while (std::getline(is, line)) {
        std::stringstream iss(line);

        std::string token;

        if (!(iss >> token)) continue;

        if (token == "name") {
            iss >> m_scene.name;
        } else if (token == "object") {}
    }
    is.close();
}

const std::string &SceneManager::GetName() {
    return m_scene.name;
}

std::vector<GameObject *> &SceneManager::GetObjects() {
    return m_scene.m_gameObjects;
}




