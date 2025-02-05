#pragma once

#include <gr-render/gRender.h>
#include <gr-math/gmath.hpp>

class PostProcessing {
public:
    PostProcessing(const iVector2 &size);
    ~PostProcessing();
    
    void resize(const iVector2 &size);

    void bind();

    void unbind();

    grr::gTexture *getTextureColor();

    void readPixel(int x, int y, void *data);

private:
    static PostProcessing *m_current;

    grr::gTexture *m_colorTexture;

    grr::gTexture *m_pickTexture;

    grr::gTexture *m_depthTexture;

    grm::u32 m_fbo;

    iVector2 m_viewport;
};
