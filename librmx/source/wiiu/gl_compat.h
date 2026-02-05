// Minimal GL compatibility header for Wii U
#pragma once

#include <cstdint>
#include <cstddef>

// Basic GL types
typedef unsigned int GLuint;
typedef int GLint;
typedef unsigned int GLenum;
typedef int GLsizei;
typedef unsigned char GLubyte;
typedef float GLfloat;
typedef void GLvoid;

typedef unsigned long GLsizeiptr;

// pointer-sized integer for offsets
#include <cstddef>
typedef intptr_t GLintptr;

// boolean type
typedef unsigned char GLboolean;
#define GL_FALSE 0
#define GL_TRUE 1


// Minimal enums used by engine
#define GL_TEXTURE_2D 0x0DE1
#define GL_RGBA 0x1908
#define GL_RGB 0x1907
#define GL_UNSIGNED_BYTE 0x1401
#define GL_UNSIGNED_SHORT 0x1403
#define GL_UNSIGNED_INT 0x1405
#define GL_TRIANGLES 0x0004
#define GL_NEAREST 0x2600
#define GL_LINEAR 0x2601
#define GL_CLAMP_TO_EDGE 0x812F
#define GL_REPEAT 0x2901
#define GL_FLOAT 0x1406
#define GL_STATIC_DRAW 0x88E4
#define GL_ARRAY_BUFFER 0x8892
#define GL_ELEMENT_ARRAY_BUFFER 0x8893
#define GL_TEXTURE_BUFFER 0x8C2A
#define GL_RGBA8 0x8058
#define GL_LUMINANCE 0x1909
#define GL_LUMINANCE_ALPHA 0x190A
// Red-format sized types
#define GL_R8UI 0x8232
#define GL_R16I 0x8233
#define GL_R16UI 0x8234
// Texture unit enums
#define GL_TEXTURE0 0x84C0
// Texture parameter enums
#define GL_TEXTURE_MAG_FILTER 0x2800
#define GL_TEXTURE_MIN_FILTER 0x2801
#define GL_TEXTURE_WRAP_S 0x2802
#define GL_TEXTURE_WRAP_T 0x2803


// Function prototypes: subset implemented in gl_compat.cpp
extern "C" {
    void wiiu_gl_initialize(int width, int height);
    void wiiu_gl_shutdown();
    void wiiu_gl_present();

    void glGenTextures(GLsizei n, GLuint* textures);
    void glDeleteTextures(GLsizei n, const GLuint* textures);
    void glBindTexture(GLenum target, GLuint texture);
    void glTexImage2D(GLenum target, GLint level, GLint internalFormat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void* pixels);
    void glTexParameteri(GLenum target, GLenum pname, GLint param);

    void glGenBuffers(GLsizei n, GLuint* buffers);
    void glDeleteBuffers(GLsizei n, const GLuint* buffers);
    void glBindBuffer(GLenum target, GLuint buffer);
    void glBufferData(GLenum target, GLsizeiptr size, const void* data, GLenum usage);

    void glEnable(GLenum cap);
    void glDisable(GLenum cap);
    void glBlendFunc(GLenum sfactor, GLenum dfactor);

    void glViewport(GLint x, GLint y, GLsizei width, GLsizei height);
    void glClearColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a);
    void glClear(GLenum mask);

    void glDrawArrays(GLenum mode, GLint first, GLsizei count);
    void glDrawElements(GLenum mode, GLsizei count, GLenum type, const void* indices);
    void glDrawElementsInstanced(GLenum mode, GLsizei count, GLenum type, const void* indices, GLsizei instancecount);
    void glDrawRangeElements(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void* indices);

    void glEnableVertexAttribArray(GLuint index);
    void glDisableVertexAttribArray(GLuint index);
    void glVertexAttribPointer(GLuint index, GLint size, GLenum type, bool normalized, GLsizei stride, const void* pointer);
    void glActiveTexture(GLenum texture);
    void glTexBuffer(GLenum target, GLenum internalFormat, GLuint buffer);
    void glTexBufferRange(GLenum target, GLint internalFormat, GLuint buffer, GLintptr offset, GLsizeiptr size);
    void glCopyTexImage2D(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
    void glCopyTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
    void glGenerateMipmap(GLenum target);
    void glReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void* pixels);
    GLboolean glIsTexture(GLuint texture);
    void* glMapBuffer(GLenum target, GLenum access);
    GLboolean glUnmapBuffer(GLenum target);
    void glBindBufferRange(GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size);

    // Vertex Array Objects
    void glGenVertexArrays(GLsizei n, GLuint* arrays);
    void glBindVertexArray(GLuint array);
    void glDeleteVertexArrays(GLsizei n, const GLuint* arrays);

    // Shader / Program API (lightweight stubs and bookkeeping)
    GLuint glCreateShader(GLenum type);
    void glShaderSource(GLuint shader, GLsizei count, const char** string, const GLint* length);
    void glCompileShader(GLuint shader);

    GLuint glCreateProgram();
    void glAttachShader(GLuint program, GLuint shader);
    void glLinkProgram(GLuint program);
    void glUseProgram(GLuint program);

    GLint glGetUniformLocation(GLuint program, const char* name);
    GLint glGetAttribLocation(GLuint program, const char* name);

    void glUniform1i(GLint location, GLint v0);
    void glUniform2iv(GLint location, GLsizei count, const GLint* value);
    void glUniform3iv(GLint location, GLsizei count, const GLint* value);
    void glUniform4iv(GLint location, GLsizei count, const GLint* value);
    void glUniform1f(GLint location, GLfloat v0);
    void glUniform2fv(GLint location, GLsizei count, const GLfloat* value);
    void glUniform3fv(GLint location, GLsizei count, const GLfloat* value);
    void glUniform4fv(GLint location, GLsizei count, const GLfloat* value);

    // Debug / Error
    GLenum glGetError();
    typedef void (*GLDEBUGPROC)(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const char* message, const void* userParam);
    void glDebugMessageCallback(GLDEBUGPROC callback, const void* userParam);
}
// Minimal GL compatibility header for Wii U
#pragma once

#include <cstdint>
#include <cstddef>

// Basic GL types
typedef unsigned int GLuint;
typedef int GLint;
typedef unsigned int GLenum;
typedef int GLsizei;
typedef unsigned char GLubyte;
typedef float GLfloat;
typedef void GLvoid;

typedef unsigned long GLsizeiptr;

// pointer-sized integer for offsets
#include <cstddef>
typedef intptr_t GLintptr;

// boolean type
typedef unsigned char GLboolean;
#define GL_FALSE 0
#define GL_TRUE 1


// Minimal enums used by engine
#define GL_TEXTURE_2D 0x0DE1
#define GL_RGBA 0x1908
#define GL_RGB 0x1907
#define GL_UNSIGNED_BYTE 0x1401
#define GL_UNSIGNED_SHORT 0x1403
#define GL_UNSIGNED_INT 0x1405
#define GL_TRIANGLES 0x0004
#define GL_NEAREST 0x2600
#define GL_LINEAR 0x2601
#define GL_CLAMP_TO_EDGE 0x812F
#define GL_REPEAT 0x2901
#define GL_FLOAT 0x1406
#define GL_STATIC_DRAW 0x88E4
#define GL_ARRAY_BUFFER 0x8892
#define GL_ELEMENT_ARRAY_BUFFER 0x8893
#define GL_TEXTURE_BUFFER 0x8C2A
#define GL_RGBA8 0x8058
#define GL_LUMINANCE 0x1909
#define GL_LUMINANCE_ALPHA 0x190A
// Red-format sized types
#define GL_R8UI 0x8232
#define GL_R16I 0x8233
#define GL_R16UI 0x8234
// Texture unit enums
#define GL_TEXTURE0 0x84C0
// Texture parameter enums
#define GL_TEXTURE_MAG_FILTER 0x2800
#define GL_TEXTURE_MIN_FILTER 0x2801
#define GL_TEXTURE_WRAP_S 0x2802
#define GL_TEXTURE_WRAP_T 0x2803


// Function prototypes: subset implemented in gl_compat.cpp
extern "C" {
    void wiiu_gl_initialize(int width, int height);
    void wiiu_gl_shutdown();
    void wiiu_gl_present();

    void glGenTextures(GLsizei n, GLuint* textures);
    void glDeleteTextures(GLsizei n, const GLuint* textures);
    void glBindTexture(GLenum target, GLuint texture);
    void glTexImage2D(GLenum target, GLint level, GLint internalFormat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void* pixels);
    void glTexParameteri(GLenum target, GLenum pname, GLint param);

    void glGenBuffers(GLsizei n, GLuint* buffers);
    void glDeleteBuffers(GLsizei n, const GLuint* buffers);
    void glBindBuffer(GLenum target, GLuint buffer);
    void glBufferData(GLenum target, GLsizeiptr size, const void* data, GLenum usage);

    void glEnable(GLenum cap);
    void glDisable(GLenum cap);
    void glBlendFunc(GLenum sfactor, GLenum dfactor);

    void glViewport(GLint x, GLint y, GLsizei width, GLsizei height);
    void glClearColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a);
    void glClear(GLenum mask);

    void glDrawArrays(GLenum mode, GLint first, GLsizei count);
    void glDrawElements(GLenum mode, GLsizei count, GLenum type, const void* indices);
    void glDrawElementsInstanced(GLenum mode, GLsizei count, GLenum type, const void* indices, GLsizei instancecount);
    void glDrawRangeElements(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void* indices);

    void glEnableVertexAttribArray(GLuint index);
    void glDisableVertexAttribArray(GLuint index);
    void glVertexAttribPointer(GLuint index, GLint size, GLenum type, bool normalized, GLsizei stride, const void* pointer);
    void glActiveTexture(GLenum texture);
    void glTexBuffer(GLenum target, GLenum internalFormat, GLuint buffer);
    void glTexBufferRange(GLenum target, GLint internalFormat, GLuint buffer, GLintptr offset, GLsizeiptr size);
    void glCopyTexImage2D(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
    void glCopyTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
    void glGenerateMipmap(GLenum target);
    void glReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void* pixels);
    GLboolean glIsTexture(GLuint texture);
    void* glMapBuffer(GLenum target, GLenum access);
    GLboolean glUnmapBuffer(GLenum target);
    void glBindBufferRange(GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size);

    // Vertex Array Objects
    void glGenVertexArrays(GLsizei n, GLuint* arrays);
    void glBindVertexArray(GLuint array);
    void glDeleteVertexArrays(GLsizei n, const GLuint* arrays);

    // Shader / Program API (lightweight stubs and bookkeeping)
    GLuint glCreateShader(GLenum type);
    void glShaderSource(GLuint shader, GLsizei count, const char** string, const GLint* length);
    void glCompileShader(GLuint shader);

    GLuint glCreateProgram();
    void glAttachShader(GLuint program, GLuint shader);
    void glLinkProgram(GLuint program);
    void glUseProgram(GLuint program);

    GLint glGetUniformLocation(GLuint program, const char* name);
    GLint glGetAttribLocation(GLuint program, const char* name);

    void glUniform1i(GLint location, GLint v0);
    void glUniform2iv(GLint location, GLsizei count, const GLint* value);
    void glUniform3iv(GLint location, GLsizei count, const GLint* value);
    void glUniform4iv(GLint location, GLsizei count, const GLint* value);
    void glUniform1f(GLint location, GLfloat v0);
    void glUniform2fv(GLint location, GLsizei count, const GLfloat* value);
    void glUniform3fv(GLint location, GLsizei count, const GLfloat* value);
    void glUniform4fv(GLint location, GLsizei count, const GLfloat* value);

    // Debug / Error
    GLenum glGetError();
    typedef void (*GLDEBUGPROC)(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const char* message, const void* userParam);
    void glDebugMessageCallback(GLDEBUGPROC callback, const void* userParam);
}
