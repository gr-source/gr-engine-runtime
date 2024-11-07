#include <iostream>
#include <memory>

#include "api.hpp"

#include "core/assetsManager.hpp"
#include "core/event.hpp"
#include "core/file.hpp"

std::unordered_map<int, grr::gShader *> shaderList;

void Initialize() {
    Console::Message(LogInfo::LogType::DEFAULT, "Update 3 XXXXXXXXXXXXXXXXXX: %f - %d", 1.5f, 80);
    std::cout << "Update 3 XXXXXXXXXXXXXXXXXX" << std::endl;

    AssetsManager::LoadShader("dynamic_one_model",
        File::GetAsset("/shader/model/dynamic_model.frag"),
        File::GetAsset("/shader/model/dynamic_one_model.vert")
    );
    
    AssetsManager::LoadShader("dynamic_anim",
        File::GetAsset("/shader/model/dynamic_model.frag"),
        File::GetAsset("/shader/model/dynamic_model_anim.vert")
    );
    
    AssetsManager::LoadShader("dynamic_multi_model",
        File::GetAsset("/shader/model/dynamic_model.frag"),
        File::GetAsset("/shader/model/dynamic_multi_model.vert")
    );
    
    AssetsManager::LoadShader("static_multi_model",
        File::GetAsset("/shader/model/dynamic_model.frag"),
        File::GetAsset("/shader/model/static_multi_model.vert")
    );
    
    AssetsManager::LoadShader("screen",
        File::GetAsset("/shader/screen.frag"),
        File::GetAsset("/shader/screen.vert")
    );
}

void Destroy() {
    std::cout << "shutdowm" << std::endl;
}














