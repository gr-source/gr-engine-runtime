#pragma once

#include <gr-render/gRender.h>
#include <gr-math/gmath.h>

class PostProcessing {
public:
    static PostProcessing *Create(const iVector2 &size);

    static void Shutdown(PostProcessing *target);

    static void Bind(PostProcessing *target);

    static void Unbind();

    static void Resize(PostProcessing *target, const iVector2 &size);

    static grr::gTexture *GetTextureColor(PostProcessing *target);

    static void Save(const std::string &path, PostProcessing *postProcessing);

    static const iVector2 &GetSize();

private:
    static PostProcessing *m_current;

    grr::gTexture *m_texture;

    grm::u32 m_rbo;

    grm::u32 m_fbo;

    iVector2 m_viewport;

    PostProcessing();
};
