#pragma once

class Timer
{
public:
    double deltaTime =
        0.0;

    Timer(double fps);

    void set_fps(double fps);

    void begin_frame();

    void end_frame() const;

private:
    double m_targetFrameTime;
    
    double lastTime =
        0.0;
};

