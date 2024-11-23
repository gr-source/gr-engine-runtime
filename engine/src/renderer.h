#pragma once

#include "Global.h"

#include "common.h"

#include <gr-render/gRender.h>

struct Line {
    Vector3 position;
    Color color;
    
    Line(const Vector3& position, const Color& color) : position(position), color(color) {}
};

class DebugDraw {
public:
	static void Initialize();

	static void Shutdown();

    static void AddLine(const Vector3& start, const Vector3& end, const Color& color);

    static void Renderer(Camera *camera);

    static bool bDebugMode;

private:
    static grr::gVertexArray *m_primitive;

    static std::vector<Line> m_points;

    static std::vector<Line> m_lines;
    
    static grr::gShader *m_shader;

    static grm::u32 m_vbo;

    static void SetupShader(grr::gShader *shader);

    static void DrawLines();
};
