#pragma once

#include "Global.h"

class ScreenRender {
public:
    ScreenRender();

    void render(Texture2D* texture);

protected:
    std::unique_ptr<BufferManager> bufferManager;
    Shader* m_shader;
};

