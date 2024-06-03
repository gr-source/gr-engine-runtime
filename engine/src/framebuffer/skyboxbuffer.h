#pragma once

#include "FrameBuffer.h"

class Skyboxbuffer : public FrameBuffer {
public:
    Skyboxbuffer(int width, int height);

    void bind() override;

    void unbind() override;

    void addSkyColor(u32 target, u32 attachment);

    Cubemap* getSkyColor();

    void addIrradiance(u32 target, u32 attachment);

    Cubemap* getIrradiance();
protected:

    std::unique_ptr<Renderbuffer> m_renderbuffer;

    std::unique_ptr<Framebuffer> m_framebuffer;

    std::unique_ptr<Cubemap> m_colorSkybox;

    std::unique_ptr<Cubemap> m_irradiance;

    std::unique_ptr<Cubemap> CreateCubemap(u32 width, u32 height);
};

