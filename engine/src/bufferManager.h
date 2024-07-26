#pragma once

#include "Global.h"

#include "gl.h"

// #define MAX_BLOCK_BUFFER 2045

class BufferManager {
public:
    enum : u16 {
        eTriangle = 0x001,
        eTriangleStrip = 0x004,
        ePoint = 0x002,
        eLine = 0x003,

        eUint = 0x1405,
        eUbyte = 0x1401,

        eVAO = 0x010,
        eVBO = 0x020,
        eEBO = 0x030,

        eSTATIC = 0x040,
        eDYNAMIC = 0x050
    };

    BufferManager();
    BufferManager(u16 type, bool elements = false);
    ~BufferManager();

    void pushBuffer(GLsizeiptr size, const void* data);
    
    void pushUpdateBuffer(GLintptr offset, GLsizeiptr size, const void* data);

    void attribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* pointer = nullptr);
    void attribPointer(GLuint index, GLint size, GLenum type, GLsizei stride, const void* pointer = nullptr);
    void attribPointer(GLuint index, GLint size, GLsizei stride, const void* pointer = nullptr);

    void applyDivisor(GLuint index, GLuint divisor);

    void pushElements(const GLsizeiptr& size, const void* data);
    void pushUpdateElements(const GLintptr& offset, const GLsizeiptr& size, const void* data);

    void bind(u16 type);
    void unbind();

    void draw(u16 mode, GLint first, GLsizei count);

    void draw(u16 mode, GLsizei count, u16 type = eUint, void* indice = nullptr);

    bool isElements;
private:
    GLuint vao;
    GLuint vbo;
    GLuint ebo;
    GLenum usage;
    GLuint mElementCount;

};

