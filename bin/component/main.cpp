#include <iostream>
#include <memory>

#include "api.hpp"

#include "core/assetsManager.hpp"
#include "core/file/file.hpp"

void Initialize() {
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

void Destroy() {}














