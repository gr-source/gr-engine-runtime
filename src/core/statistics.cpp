#include "gr-engine/core/statistics.hpp"

usize Statistics::m_drawRenderCount = 0;

usize Statistics::m_drawCallCount = 0;

usize Statistics::m_totalRender = 0;

gFrameTime Statistics::FrameTime[FrameTimeID_Count];

gFrameTime::gFrameTime() : fps(0.0f), delta(0.0f), maxFPS(0.0f), minFPS(0.0f), minDelta(0.0f), maxDelta(0.0f), elapsed(0.0f)
{}

void gFrameTime::setFPS(float value)
{
    fps = value;

    maxFPS      = std::max(maxFPS, value);
    minFPS      = std::min(minFPS, value);
}

void gFrameTime::setDelta(float value)
{
    delta = value;

    maxDelta    = std::max(maxDelta, value);
    minDelta    = std::min(maxDelta, value);

    elapsed += value;
    if (elapsed >= 1.0f)
    {
        elapsed = 0.0f;

        setFPS(1.0f / value);
    }
}


