#include <iostream>
#include <memory>

#include "api.hpp"

#include "core/assetsManager.hpp"
#include "core/event.hpp"
#include "core/file.hpp"

void Initialize() {
    Console::log(LogInfo::LogType::DEFAULT, "KKKKKKKKKKKK");
    std::cout << "mmmmmmmmmmmmmmm" << std::endl;
}

void Destroy() {
    std::cout << "shutdowm" << std::endl;
    Event::Shutdown();

    // ComponentRegistry::GetPool().clear();
}

enum ShaderType : int {
    ShaderType_Dynamic = 0,
    ShaderType_Dynamic_Anim = 1,
    ShaderType_Dynamic_Multi_Model = 2,
    ShaderType_Static_Multi_Model = 3,
    ShaderType_Screen = 4,
};

std::unordered_map<int, grr::gShader *> GetShaders() {
    return {
        {
            ShaderType_Dynamic,
            AssetsManager::LoadShader("dynamic_one_model",
                "/home/grsource/Documentos/AppProjects/gr-component/lib/shader/model/dynamic_model.frag",
                "/home/grsource/Documentos/AppProjects/gr-component/lib/shader/model/dynamic_one_model.vert"
            )
        },
        {
            ShaderType_Dynamic_Anim,
            AssetsManager::LoadShader("dynamic_anim",
                "/home/grsource/Documentos/AppProjects/gr-component/lib/shader/model/dynamic_model.frag",
                "/home/grsource/Documentos/AppProjects/gr-component/lib/shader/model/dynamic_model_anim.vert"
            )
        },
        {
            ShaderType_Dynamic_Multi_Model,
            AssetsManager::LoadShader("dynamic_multi_model",
                "/home/grsource/Documentos/AppProjects/gr-component/lib/shader/model/dynamic_model.frag",
                "/home/grsource/Documentos/AppProjects/gr-component/lib/shader/model/dynamic_multi_model.vert"
            )
        },
        {
            ShaderType_Static_Multi_Model,
            AssetsManager::LoadShader("static_multi_model",
                "/home/grsource/Documentos/AppProjects/gr-component/lib/shader/model/dynamic_model.frag",
                "/home/grsource/Documentos/AppProjects/gr-component/lib/shader/model/static_multi_model.vert"
            )
        },
        {
            ShaderType_Screen,
            AssetsManager::LoadShader("screen",
                "/home/grsource/Documentos/AppProjects/gr-component/lib/shader/screen.frag",
                "/home/grsource/Documentos/AppProjects/gr-component/lib/shader/screen.vert"
            )
        },
    };
}














