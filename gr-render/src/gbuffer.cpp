#include "gbuffer.h"

#include "gl.h"

namespace grr {
    gbuffer* gbuffer::instance = nullptr;

    gbuffer* gbuffer::Create() {
        gbuffer* buffer = (gbuffer*)GMALLOC(sizeof(gbuffer));
        buffer->index = (u32*)GCALLOC(MAX_INDEX, sizeof(u32));
        buffer->indexCount = 0;

        GL_CHECK(glGenVertexArrays(1, &buffer->vao));

        return buffer;
    }

    u32& gbuffer::CreateBuffer(u8 type) {
        u32& index = (type == eVBO ? instance->index[GBUFFER_INDEX_VBO] : instance->index[GBUFFER_INDEX_EBO]);

        GL_CHECK(glGenBuffers(1, &index));

        switch (type) {
        case eVBO:
            GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, index));
            break;
        case eEBO:
            GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index));
            break;
        default:
            GEXCEPTION("void gbuffer::CreateBuffer(u8 type)");
        }
        return index;
    }

    void gbuffer::BindBuffer(u8 type) {
        switch (type) {
        case eVBO:
            GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, instance->index[GBUFFER_INDEX_VBO]));
            break;
        case eEBO:
            GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, instance->index[GBUFFER_INDEX_EBO]));
            break;
        default:
            GEXCEPTION("void gbuffer::BindBuffer(u8 type)");
        }
    }

    void gbuffer::SetBufferData(u8 mode, u8 usage, ptrsize size, const void*data) {
        switch (mode) {
        case eVBO:
            GL_CHECK(glBufferData(GL_ARRAY_BUFFER, size, data, (usage == eSTATIC ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW)));
        case eEBO:
            GL_CHECK(glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, (usage == eSTATIC ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW)));
        default:
            GEXCEPTION("void gbuffer::SetBufferData(u8 mode, u8 usage, ptrsize size, void *data)");
        }
    }

    void gbuffer::SetBufferSubData(u8 mode, u32 offset, ptrsize size, const void*data) {
        switch (mode) {
        case eVBO:
            GL_CHECK(glBufferSubData(GL_ARRAY_BUFFER, offset, size, data));
        case eEBO:
            GL_CHECK(glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, offset, size, data));
        default:
            GEXCEPTION("void gbuffer::SetBufferSubData(u8 mode, u32 offset, ptrsize size, const void*data)");
        }
    }

    void gbuffer::AddAttribPointer(u32 index, int size, u8 type, bool normalized, ptrsize stride, const void*pointer) {
        switch (type) {
        case eFLOAT:
            GL_CHECK(glVertexAttribPointer(index, size, GL_FLOAT, normalized, stride, pointer));
            break;
        default:
            GEXCEPTION("void gbuffer::AddAttribPointer(u32 index, int size, u8 type, bool normalized, ptrsize stride, const void*pointer)");
        }
        GL_CHECK(glEnableVertexAttribArray(index));
    }

    void gbuffer::AddAttribIPointer(u32 index, int size, u8 type, ptrsize stride, const void*pointer) {
        switch (type) {
        case eFLOAT:
            GL_CHECK(glVertexAttribIPointer(index, size, GL_FLOAT, stride, pointer));
            break;
        case eINT:
            GL_CHECK(glVertexAttribIPointer(index, size, GL_INT, stride, pointer));
            break;
        default:
            GEXCEPTION("void gbuffer::AddAttribPointer(u32 index, int size, u8 type, bool normalized, ptrsize stride, const void*pointer)");
        }
        GL_CHECK(glEnableVertexAttribArray(index));
    }

    void gbuffer::drawArray(u8 mode, int first, int count) {
        switch (mode) {
        case eTRIANGLE:
            GL_CHECK(glDrawArrays(GL_TRIANGLES, first, count));
            break;
        case eLINES:
            GL_CHECK(glDrawArrays(GL_LINES, first, count));
            break;
        case ePOINTS:
            GL_CHECK(glDrawArrays(GL_POINTS, first, count));
            break;
        default:
            GEXCEPTION("void gbuffer::drawArray(u8 mode, int first, int count)");
        }
    }

    void gbuffer::drawElements(u8 mode, int count, u32 type, const void*indices) {
        switch (mode) {
        case eTRIANGLE:
            GL_CHECK(glDrawElements(GL_TRIANGLES, count, (type == eUBYTE ? GL_UNSIGNED_BYTE : GL_UNSIGNED_INT), indices));
            break;
        case eLINES:
            GL_CHECK(glDrawElements(GL_LINES, count, (type == eUBYTE ? GL_UNSIGNED_BYTE : GL_UNSIGNED_INT), indices));
            break;
        case ePOINTS:
            GL_CHECK(glDrawElements(GL_POINTS, count, (type == eUBYTE ? GL_UNSIGNED_BYTE : GL_UNSIGNED_INT), indices));
            break;
        default:
            GEXCEPTION("void gbuffer::drawArray(u8 mode, int first, int count)");
        }
    }

    void gbuffer::bind() {
        gbuffer::instance = this;
        GL_CHECK(glBindVertexArray(vao));
    }

    void gbuffer::unbind() {
        gbuffer::instance = nullptr;
        GL_CHECK(glBindVertexArray(0));
    }

    void gbuffer::destroy() {
        GL_CHECK(glDeleteBuffers(MAX_INDEX, index));
        GL_CHECK(glDeleteVertexArrays(1, &vao));
        GFREE(this);
    }
};