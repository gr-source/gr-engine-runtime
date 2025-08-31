#pragma once

#include "gr-base/script/default.hpp"

class RenderManager {
public:
    static void PreRender(const Camera *camera);

    static void Render();

    static void PosRender();

private:
    static RenderManager &GetInstance();

    RenderManager();
    ~RenderManager();

    void preRender(const Camera *camera);

    void posRender();

    void render();
};
