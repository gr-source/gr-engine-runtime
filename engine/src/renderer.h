#pragma once

#include "Global.h"

#include "common.h"

#include "bufferManager.h"

struct Line {
    Vector3 position;
    Color color;
    
    Line(const Vector3& position, const Color& color) : position(position), color(color) {}
};

struct SpriteRender {
    Sprite* m_sprite;
    Matrix4x4 model;
};

class DebugDraw {
public:
	static void initialize();

    static void addLine(const Vector3& start, const Vector3& end, const Color& color);

    static void addPoint(const Vector3& position, const Color& color);

    static void addSprite(const Vector3& position, Sprite* sprite);

    static const bool isvalid();

    static const bool hasLines();

	static void drawLines();

    static const bool hasPoints();

    static void drawPoints();

    static const bool hassprites();

    static void drawsprites();

    static void bind();

    static void unbind();

    static void Renderer();

protected:
	static std::unique_ptr<BufferManager> m_primitivebuffer;

	static std::unique_ptr<BufferManager> m_spritebuffer;

    static std::vector<SpriteRender> m_sprites;

    static std::vector<Line> m_lines;
    
    static std::vector<Line> m_points;

    static Shader* m_spriteRender;

	static Shader* m_shader;

    static void SetupSpriteRender();

};
