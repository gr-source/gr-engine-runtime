#pragma once

#include "common.h"

namespace gr {
    enum RenderState : u8 {
        GR_FALSE            = 0x000,
        GR_TRUE             = 0x001,
        GR_BACKGROUND_COLOR = 0x002,
        GR_BACKGROUND       = 0x003,
        GR_DEPTH_BUFFER     = 0x004,
        GR_COLOR_BUFFER     = 0x005,
    };

    enum PrimitiveType {
        PrimitiveType_Points = 0,
        PrimitiveType_Lines = 1,
        PrimitiveType_LineLoop = 2,
        PrimitiveType_LineStrip = 3,
        PrimitiveType_Triangles = 4,
        PrimitiveType_TriangleStrip = 5,
        PrimitiveType_TriangleFan = 6
    };

    struct Vertex2D {
        Vector2 position;
    };

    struct Vertex3D {
        Vector3 position;
    };

    class gRender {
    public:

        static void SetRenderState(RenderState state, void* value);

        static void SetRenderState(RenderState state, u8 value);

        static const std::string getRenderStateName(RenderState state);

        static const bool OpenGLInit();

        static void Render2D();

        static void RenderPrimitive2D(PrimitiveType primitive, u32 numVertice, void* vertice);

        static void Render3D();

        static void RenderPrimitive3D(PrimitiveType primitive, u32 numVertice, void* vertice);

        static void RenderIndexedPrimitive3D(PrimitiveType primitive, u32 numVertice, void* vertice, u32 numIndice, void* indice);

        static void OpenGLShutsown();
    private:
        static std::unordered_map<PrimitiveType, u32> m_primitiveMap;

        static gShader* m_shader2D;
        static gShader* m_shader3D;
    };
}