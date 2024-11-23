#pragma once

#include <gr-math/gmath.hpp>

struct FrameTime {
    float fps;
    float delta;
};

class Statistics {
public:
    static grm::size m_drawRenderCount;

    static grm::size m_drawCallCount;

    static grm::size m_totalRender;

    static FrameTime m_threadFrame;

    static FrameTime m_scriptFrame;

    static FrameTime m_sceneFrame;

    static FrameTime m_guiFrame;

private:
};

