#pragma once

#include <gr-render/gRender.h>

#include "Global.h"

class ScreenRender {
public:
    ScreenRender();

    void render(grr::gTexture *texture);

protected:
    std::unique_ptr<BufferManager> bufferManager;
    Shader* m_shader;
};

