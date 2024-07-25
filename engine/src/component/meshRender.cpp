#include "meshRender.hpp"

#include "../scriptManager/scriptManager.hpp"

std::vector<GameObject *> &MeshRender::GetGameObjectFromRender() {
    return ScriptManager::GetGameObjectsFromRender();
}
