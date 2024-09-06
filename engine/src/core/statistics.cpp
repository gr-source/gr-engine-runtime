#include "statistics.hpp"

grm::size Statistics::m_drawRenderCount = 0;

grm::size Statistics::m_drawCallCount = 0;

grm::size Statistics::m_totalRender = 0;

FrameTime Statistics::m_threadFrame = {0.0f, 0.0f};

FrameTime Statistics::m_scriptFrame = {0.0f, 0.0f};

FrameTime Statistics::m_sceneFrame = {0.0f, 0.0f};

FrameTime Statistics::m_guiFrame = {0.0f, 0.0f};



