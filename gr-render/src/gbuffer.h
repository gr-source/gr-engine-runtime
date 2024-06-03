#pragma once

#include "common.h"

#define MAX_INDEX 2
#define GBUFFER_INDEX_VBO 0
#define GBUFFER_INDEX_EBO 1

namespace grr {
    struct gbuffer {
        enum : u32 {
            eFALSE   = 0x000,
            eTRUE    = 0x001,
            eSTATIC    = 0x002,
            eDYNAMIC = 0x003,
            eVBO     = 0x0010,
            eEBO     = 0x0011,
            eFLOAT   = 0x0020,
            eINT     = 0x0021,

            eUBYTE     = 0x0030,
            eUINT     = 0x0031,

            eTRIANGLE     = 0x0040,
            eLINES     = 0x0041,
            ePOINTS     = 0x0042
        };

        u32 vao;

        u32* index;
        u32 indexCount;

        static gbuffer* instance;

        static gbuffer* Create();

        static u32& CreateBuffer(u8 type);

        static void BindBuffer(u8 type);

        static void SetBufferData(u8 mode, u8 usage, ptrsize size, const void* data);

        static void SetBufferSubData(u8 mode, u32 offset, ptrsize size, const void* data);

        static void AddAttribPointer(u32 index, int size, u8 type, bool normalized, ptrsize stride, const void* pointer);

        static void AddAttribIPointer(u32 index, int size, u8 type, ptrsize stride, const void* pointer);

        static void drawArray(u8 mode, int first, int count);

        static void drawElements(u8 mode, int count, u32 type, const void* indices);

        void bind();

        void unbind();

        void destroy();

    };
};

