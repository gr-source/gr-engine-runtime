#include "bufferManager.h"

BufferManager::BufferManager() : vao(-1), vbo(-1), ebo(-1), usage(GL_STATIC_DRAW), isElements(false) {}

BufferManager::BufferManager(u16 type, bool elements) : vao(-1), vbo(-1), ebo(-1), isElements(elements) {
    switch (type) {
    case eSTATIC:
        usage = GL_STATIC_DRAW;
        break;
    case eDYNAMIC:
        usage = GL_DYNAMIC_DRAW;
        break;
    default:
        assert(false && "BufferManager type status invalid");
    }
    GL_CHECK(glGenVertexArrays(1, &vao));
    GL_CHECK(glGenBuffers(1, &vbo));

    if (elements) {
        GL_CHECK(glGenBuffers(1, &ebo));
    }
}

BufferManager::~BufferManager() {
    if (vao)
        GL_CHECK(glDeleteVertexArrays(1, &vao));

    if (vbo)
        GL_CHECK(glDeleteBuffers(1, &vbo));

    if (ebo)
        GL_CHECK(glDeleteBuffers(1, &ebo));
}

void BufferManager::setup(u16 type, bool elements) {
    switch (type) {
    case eSTATIC:
        usage = GL_STATIC_DRAW;
        break;
    case eDYNAMIC:
        usage = GL_DYNAMIC_DRAW;
        break;
    default:
        assert(false && "BufferManager type status invalid");
    }
    GL_CHECK(glGenVertexArrays(1, &vao));
    GL_CHECK(glGenBuffers(1, &vbo));

    if (elements) {
        GL_CHECK(glGenBuffers(1, &ebo));
    }
}

void BufferManager::pushBuffer(GLsizeiptr size, const void* data) {
    GL_CHECK(glBufferData(GL_ARRAY_BUFFER, size, data, usage));
}

void BufferManager::pushUpdateBuffer(GLintptr offset, GLsizeiptr size, const void* data) {
    GL_CHECK(glBufferSubData(GL_ARRAY_BUFFER, offset, size, data));
}

void BufferManager::attribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* pointer) {
    GL_CHECK(glVertexAttribPointer(index, size, type, normalized, stride, pointer));
    GL_CHECK(glEnableVertexAttribArray(index));
}

void BufferManager::attribPointer(GLuint index, GLint size, GLenum type, GLsizei stride, const void* pointer) {
    GL_CHECK(glVertexAttribIPointer(index, size, type, stride, pointer));
    GL_CHECK(glEnableVertexAttribArray(index));
}

void BufferManager::attribPointer(GLuint index, GLint size, GLsizei stride, const void *pointer) {
    GL_CHECK(glVertexAttribPointer(index, size, GL_FLOAT, GL_FALSE, stride, pointer));
    GL_CHECK(glEnableVertexAttribArray(index));
}

void BufferManager::applyDivisor(GLuint index, GLuint divisor) {
    GL_CHECK(glVertexAttribDivisor(index, divisor));
}

void BufferManager::resizeElements(GLsizeiptr size) {
    GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo));
    GL_CHECK(glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, nullptr, usage));
}

void BufferManager::pushElements(const GLsizeiptr& size, const void* data) {
    GL_CHECK(glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, usage));
    mElementCount = size;
}

void BufferManager::pushUpdateElements(const GLintptr& offset, const GLsizeiptr& size, const void* data) {
    GL_CHECK(glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, offset, size, data));
}

void BufferManager::bind(u16 type) {
    switch (type) {
    case eVAO:
        GL_CHECK(glBindVertexArray(vao));
        break;
    case eVBO:
        GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, vbo));
        break;
    case eEBO:
        GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo));
        break;
    default:
        assert(false && " [ BufferManager::bind ] type do not suported");
    }
}

void BufferManager::unbind() {
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void BufferManager::draw(u16 mode, GLint first, GLsizei count) {
    switch (mode) {
    case eLine:
        return GL_CHECK(glDrawArrays(GL_LINES, first, count));
    case ePoint:
        return GL_CHECK(glDrawArrays(GL_POINTS, first, count));
    case eTriangle:
        return GL_CHECK(glDrawArrays(GL_TRIANGLES, first, count));
    case eTriangleStrip:
        return GL_CHECK(glDrawArrays(GL_TRIANGLE_STRIP, first, count));
    default:
        assert(false && "[ BufferManager::draw ] draw type invalid");
        break;
    }
}

void BufferManager::draw(u16 mode, GLsizei count, u16 type, void* indice) {
    GLenum itype = (type == eUint ? GL_UNSIGNED_INT : GL_UNSIGNED_BYTE);

    switch (mode) {
    case eTriangle:
        return GL_CHECK(glDrawElements(GL_TRIANGLES, count, itype, indice));
    case eTriangleStrip:
        return GL_CHECK(glDrawElements(GL_TRIANGLE_STRIP, count, itype, indice));
    case eLine:
        return GL_CHECK(glDrawElements(GL_LINES, count, itype, indice));
    case ePoint:
        return GL_CHECK(glDrawElements(GL_POINTS, count, itype, indice));
    default:
        assert(false && "[ BufferManager::draw ] draw type invalid");
        break;
    }
}


