#pragma once

#include <gr-render/gRender.h>
#include <gr-math/gmath.h>

class PostProcessing {
public:
    PostProcessing(const iVector2 &size);
    ~PostProcessing();
    
    void resize(const iVector2 &size);

    void bind();

    void unbind();

    grr::gTexture *getTextureColor();

    // static void Save(const std::string &path, PostProcessing *postProcessing);

    // static const iVector2 &GetSize();

    // static Vector3 GetPixelInfo(PostProcessing *target);

private:
    static PostProcessing *m_current;

    grr::gTexture *m_colorTexture;

    grr::gTexture *m_depthTexture;

    grm::u32 m_fbo;

    iVector2 m_viewport;
};
