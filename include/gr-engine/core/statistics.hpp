#pragma once

#include <gr-math/gmath.hpp>

typedef struct gFrameTime
{
    float fps;
    
    float delta;

    float maxFPS;

    float minFPS;

    float maxDelta;

    float minDelta;

    float elapsed;

    gFrameTime();

    void setFPS(float value);

    void setDelta(float value);
} gFrameTime;

enum FrameTimeID
{
    FrameTimeID_MainThread  = 1,
    FrameTimeID_Script      = 2,
    FrameTimeID_Gui         = 3,
    FrameTimeID_Count,
};

class Statistics {
public:
    static usize m_drawRenderCount;

    static usize m_drawCallCount;

    static usize m_totalRender;

    static gFrameTime FrameTime[FrameTimeID_Count];
};

