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
typedef double GLdouble;
typedef void GLvoid;
typedef char GLchar;
typedef unsigned long GLsizeiptr;
typedef intptr_t GLintptr;
typedef unsigned char GLboolean;
typedef unsigned short GLushort;

#define GL_FALSE 0
#define GL_TRUE 1

// ---------- Enums ----------

// Primitive types
#define GL_POINTS         0x0000
#define GL_LINES          0x0001
#define GL_LINE_STRIP     0x0003
#define GL_TRIANGLES      0x0004
#define GL_TRIANGLE_STRIP 0x0005
#define GL_TRIANGLE_FAN   0x0006

// Texture targets
#define GL_TEXTURE_1D     0x0DE0
#define GL_TEXTURE_2D     0x0DE1
#define GL_TEXTURE_3D     0x806F
#define GL_TEXTURE_CUBE_MAP 0x8513

// Pixel formats
#define GL_RED            0x1903
#define GL_RG             0x8227
#define GL_RGB            0x1907
#define GL_RGBA           0x1908
#define GL_BGR            0x80E0
#define GL_BGRA           0x80E1
#define GL_LUMINANCE      0x1909
#define GL_LUMINANCE_ALPHA 0x190A
#define GL_ALPHA          0x1906
#define GL_DEPTH_COMPONENT 0x1902

// Pixel types
#define GL_UNSIGNED_BYTE  0x1401
#define GL_BYTE           0x1400
#define GL_UNSIGNED_SHORT 0x1403
#define GL_SHORT          0x1402
#define GL_UNSIGNED_INT   0x1405
#define GL_INT            0x1404
#define GL_FLOAT          0x1406
#define GL_HALF_FLOAT     0x140B

// Sized internal formats
#define GL_R8             0x8229
#define GL_R16            0x822A
#define GL_RG8            0x822B
#define GL_RG16           0x822C
#define GL_R8UI           0x8232
#define GL_R16I           0x8233
#define GL_R16UI          0x8234
#define GL_R32I           0x8235
#define GL_R32UI          0x8236
#define GL_RGBA8          0x8058
#define GL_RGBA16         0x805B
#define GL_RGBA16F        0x881A
#define GL_RGBA32F        0x8814
#define GL_RGB8           0x8051
#define GL_SRGB8_ALPHA8   0x8C43

// Texture filter/wrap
#define GL_NEAREST        0x2600
#define GL_LINEAR         0x2601
#define GL_NEAREST_MIPMAP_NEAREST 0x2700
#define GL_LINEAR_MIPMAP_NEAREST  0x2701
#define GL_NEAREST_MIPMAP_LINEAR  0x2702
#define GL_LINEAR_MIPMAP_LINEAR   0x2703
#define GL_CLAMP_TO_EDGE  0x812F
#define GL_REPEAT         0x2901
#define GL_MIRRORED_REPEAT 0x8370
#define GL_CLAMP_TO_BORDER 0x812D
#define GL_TEXTURE_MAG_FILTER 0x2800
#define GL_TEXTURE_MIN_FILTER 0x2801
#define GL_TEXTURE_WRAP_S 0x2802
#define GL_TEXTURE_WRAP_T 0x2803
#define GL_TEXTURE_WRAP_R 0x8072
#define GL_TEXTURE_MAX_LEVEL 0x813D

// Texture units
#define GL_TEXTURE0       0x84C0
#define GL_TEXTURE1       0x84C1
#define GL_TEXTURE2       0x84C2
#define GL_TEXTURE3       0x84C3
#define GL_TEXTURE4       0x84C4
#define GL_TEXTURE5       0x84C5
#define GL_TEXTURE6       0x84C6
#define GL_TEXTURE7       0x84C7

// Buffer targets
#define GL_ARRAY_BUFFER          0x8892
#define GL_ELEMENT_ARRAY_BUFFER  0x8893
#define GL_TEXTURE_BUFFER        0x8C2A
#define GL_UNIFORM_BUFFER        0x8A11
#define GL_PIXEL_PACK_BUFFER     0x88EB
#define GL_PIXEL_UNPACK_BUFFER   0x88EC

// Buffer usage
#define GL_STATIC_DRAW    0x88E4
#define GL_DYNAMIC_DRAW   0x88E8
#define GL_STREAM_DRAW    0x88E0

// Buffer access
#define GL_READ_ONLY      0x88B8
#define GL_WRITE_ONLY     0x88B9
#define GL_READ_WRITE     0x88BA

// Map buffer range flags
#define GL_MAP_READ_BIT              0x0001
#define GL_MAP_WRITE_BIT             0x0002
#define GL_MAP_INVALIDATE_RANGE_BIT  0x0004
#define GL_MAP_INVALIDATE_BUFFER_BIT 0x0008
#define GL_MAP_FLUSH_EXPLICIT_BIT    0x0010
#define GL_MAP_UNSYNCHRONIZED_BIT    0x0020

// Capability flags
#define GL_BLEND          0x0BE2
#define GL_DEPTH_TEST     0x0B71
#define GL_SCISSOR_TEST   0x0C11
#define GL_STENCIL_TEST   0x0B90
#define GL_CULL_FACE      0x0B44
#define GL_DITHER         0x0BD0
#define GL_MULTISAMPLE    0x809D

// Blend factors
#define GL_ZERO                 0
#define GL_ONE                  1
#define GL_SRC_COLOR            0x0300
#define GL_ONE_MINUS_SRC_COLOR  0x0301
#define GL_SRC_ALPHA            0x0302
#define GL_ONE_MINUS_SRC_ALPHA  0x0303
#define GL_DST_ALPHA            0x0304
#define GL_ONE_MINUS_DST_ALPHA  0x0305
#define GL_DST_COLOR            0x0306
#define GL_ONE_MINUS_DST_COLOR  0x0307
#define GL_SRC_ALPHA_SATURATE   0x0308

// Blend equations
#define GL_FUNC_ADD             0x8006
#define GL_FUNC_SUBTRACT        0x800A
#define GL_FUNC_REVERSE_SUBTRACT 0x800B
#define GL_MIN                  0x8007
#define GL_MAX                  0x8008

// Clear bits
#define GL_COLOR_BUFFER_BIT   0x00004000
#define GL_DEPTH_BUFFER_BIT   0x00000100
#define GL_STENCIL_BUFFER_BIT 0x00000400

// Shader types
#define GL_VERTEX_SHADER   0x8B31
#define GL_FRAGMENT_SHADER 0x8B30
#define GL_GEOMETRY_SHADER 0x8DD9

// Shader/Program query
#define GL_COMPILE_STATUS  0x8B81
#define GL_LINK_STATUS     0x8B82
#define GL_INFO_LOG_LENGTH 0x8B84
#define GL_SHADER_TYPE     0x8B4F
#define GL_DELETE_STATUS   0x8B80
#define GL_ATTACHED_SHADERS 0x8B85
#define GL_ACTIVE_UNIFORMS 0x8B86
#define GL_ACTIVE_ATTRIBUTES 0x8B89

// Framebuffer
#define GL_FRAMEBUFFER                0x8D40
#define GL_READ_FRAMEBUFFER           0x8CA8
#define GL_DRAW_FRAMEBUFFER           0x8CA9
#define GL_RENDERBUFFER               0x8D41
#define GL_COLOR_ATTACHMENT0          0x8CE0
#define GL_COLOR_ATTACHMENT1          0x8CE1
#define GL_COLOR_ATTACHMENT2          0x8CE2
#define GL_COLOR_ATTACHMENT3          0x8CE3
#define GL_DEPTH_ATTACHMENT           0x8D00
#define GL_STENCIL_ATTACHMENT         0x8D20
#define GL_DEPTH_STENCIL_ATTACHMENT   0x821A
#define GL_FRAMEBUFFER_COMPLETE       0x8CD5
#define GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT 0x8CD6
#define GL_FRAMEBUFFER_UNDEFINED      0x8219

// Pixel store
#define GL_PACK_ALIGNMENT    0x0D05
#define GL_UNPACK_ALIGNMENT  0x0CF5
#define GL_UNPACK_ROW_LENGTH 0x0CF2

// Error codes
#define GL_NO_ERROR          0
#define GL_INVALID_ENUM      0x0500
#define GL_INVALID_VALUE     0x0501
#define GL_INVALID_OPERATION 0x0502
#define GL_OUT_OF_MEMORY     0x0505

// Get params
#define GL_MAX_TEXTURE_SIZE  0x0D33
#define GL_VIEWPORT          0x0BA2
#define GL_VERSION           0x1F02
#define GL_RENDERER          0x1F01
#define GL_VENDOR            0x1F00
#define GL_EXTENSIONS        0x1F03
#define GL_SHADING_LANGUAGE_VERSION 0x8B8C
#define GL_NUM_EXTENSIONS    0x821D
#define GL_MAJOR_VERSION     0x821B
#define GL_MINOR_VERSION     0x821C

// Face / Polygon
#define GL_FRONT             0x0404
#define GL_BACK              0x0405
#define GL_FRONT_AND_BACK    0x0408
#define GL_CW                0x0900
#define GL_CCW               0x0901

// Depth functions
#define GL_NEVER             0x0200
#define GL_LESS              0x0201
#define GL_EQUAL             0x0202
#define GL_LEQUAL            0x0203
#define GL_GREATER           0x0204
#define GL_NOTEQUAL          0x0205
#define GL_GEQUAL            0x0206
#define GL_ALWAYS            0x0207

// ---------- Function prototypes ----------
extern "C" {

    // Wii U lifecycle
    void wiiu_gl_initialize(int width, int height);
    void wiiu_gl_shutdown();
    void wiiu_gl_present();

    // Textures
    void glGenTextures(GLsizei n, GLuint* textures);
    void glDeleteTextures(GLsizei n, const GLuint* textures);
    void glBindTexture(GLenum target, GLuint texture);
    void glTexImage2D(GLenum target, GLint level, GLint internalFormat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void* pixels);
    void glTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void* pixels);
    void glTexParameteri(GLenum target, GLenum pname, GLint param);
    void glTexParameterf(GLenum target, GLenum pname, GLfloat param);
    GLboolean glIsTexture(GLuint texture);
    void glActiveTexture(GLenum texture);
    void glTexBuffer(GLenum target, GLenum internalFormat, GLuint buffer);
    void glTexBufferRange(GLenum target, GLint internalFormat, GLuint buffer, GLintptr offset, GLsizeiptr size);
    void glCopyTexImage2D(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
    void glCopyTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
    void glGenerateMipmap(GLenum target);
    void glReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void* pixels);
    void glPixelStorei(GLenum pname, GLint param);

    // Buffers
    void glGenBuffers(GLsizei n, GLuint* buffers);
    void glDeleteBuffers(GLsizei n, const GLuint* buffers);
    void glBindBuffer(GLenum target, GLuint buffer);
    void glBufferData(GLenum target, GLsizeiptr size, const void* data, GLenum usage);
    void glBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, const void* data);
    void* glMapBuffer(GLenum target, GLenum access);
    void* glMapBufferRange(GLenum target, GLintptr offset, GLsizeiptr length, GLuint access);
    GLboolean glUnmapBuffer(GLenum target);
    void glBindBufferRange(GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size);
    void glBindBufferBase(GLenum target, GLuint index, GLuint buffer);

    // State
    void glEnable(GLenum cap);
    void glDisable(GLenum cap);
    GLboolean glIsEnabled(GLenum cap);
    void glBlendFunc(GLenum sfactor, GLenum dfactor);
    void glBlendFuncSeparate(GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha);
    void glBlendEquation(GLenum mode);
    void glBlendEquationSeparate(GLenum modeRGB, GLenum modeAlpha);
    void glDepthFunc(GLenum func);
    void glDepthMask(GLboolean flag);
    void glColorMask(GLboolean r, GLboolean g, GLboolean b, GLboolean a);
    void glCullFace(GLenum mode);
    void glFrontFace(GLenum mode);
    void glLineWidth(GLfloat width);
    void glPointSize(GLfloat size);
    void glPolygonOffset(GLfloat factor, GLfloat units);
    void glScissor(GLint x, GLint y, GLsizei width, GLsizei height);

    // Viewport and clear
    void glViewport(GLint x, GLint y, GLsizei width, GLsizei height);
    void glClearColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a);
    void glClear(GLenum mask);
    void glClearDepth(GLdouble depth);

    // Draw calls
    void glDrawArrays(GLenum mode, GLint first, GLsizei count);
    void glDrawElements(GLenum mode, GLsizei count, GLenum type, const void* indices);
    void glDrawElementsInstanced(GLenum mode, GLsizei count, GLenum type, const void* indices, GLsizei instancecount);
    void glDrawRangeElements(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void* indices);
    void glDrawArraysInstanced(GLenum mode, GLint first, GLsizei count, GLsizei instancecount);

    // Vertex attributes
    void glEnableVertexAttribArray(GLuint index);
    void glDisableVertexAttribArray(GLuint index);
    void glVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* pointer);
    void glVertexAttribIPointer(GLuint index, GLint size, GLenum type, GLsizei stride, const void* pointer);
    void glVertexAttribDivisor(GLuint index, GLuint divisor);

    // VAO
    void glGenVertexArrays(GLsizei n, GLuint* arrays);
    void glBindVertexArray(GLuint array);
    void glDeleteVertexArrays(GLsizei n, const GLuint* arrays);

    // Shader / Program API
    GLuint glCreateShader(GLenum type);
    void glDeleteShader(GLuint shader);
    void glShaderSource(GLuint shader, GLsizei count, const GLchar* const* string, const GLint* length);
    void glCompileShader(GLuint shader);
    void glGetShaderiv(GLuint shader, GLenum pname, GLint* params);
    void glGetShaderInfoLog(GLuint shader, GLsizei bufSize, GLsizei* length, GLchar* infoLog);

    GLuint glCreateProgram();
    void glDeleteProgram(GLuint program);
    void glAttachShader(GLuint program, GLuint shader);
    void glDetachShader(GLuint program, GLuint shader);
    void glLinkProgram(GLuint program);
    void glUseProgram(GLuint program);
    void glValidateProgram(GLuint program);
    void glGetProgramiv(GLuint program, GLenum pname, GLint* params);
    void glGetProgramInfoLog(GLuint program, GLsizei bufSize, GLsizei* length, GLchar* infoLog);
    void glBindAttribLocation(GLuint program, GLuint index, const GLchar* name);

    GLint glGetUniformLocation(GLuint program, const GLchar* name);
    GLint glGetAttribLocation(GLuint program, const GLchar* name);

    // Uniforms
    void glUniform1i(GLint location, GLint v0);
    void glUniform1f(GLint location, GLfloat v0);
    void glUniform2i(GLint location, GLint v0, GLint v1);
    void glUniform2f(GLint location, GLfloat v0, GLfloat v1);
    void glUniform3i(GLint location, GLint v0, GLint v1, GLint v2);
    void glUniform3f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
    void glUniform4i(GLint location, GLint v0, GLint v1, GLint v2, GLint v3);
    void glUniform4f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
    void glUniform1iv(GLint location, GLsizei count, const GLint* value);
    void glUniform2iv(GLint location, GLsizei count, const GLint* value);
    void glUniform3iv(GLint location, GLsizei count, const GLint* value);
    void glUniform4iv(GLint location, GLsizei count, const GLint* value);
    void glUniform1fv(GLint location, GLsizei count, const GLfloat* value);
    void glUniform2fv(GLint location, GLsizei count, const GLfloat* value);
    void glUniform3fv(GLint location, GLsizei count, const GLfloat* value);
    void glUniform4fv(GLint location, GLsizei count, const GLfloat* value);
    void glUniformMatrix2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
    void glUniformMatrix3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
    void glUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);

    // Framebuffer
    void glGenFramebuffers(GLsizei n, GLuint* framebuffers);
    void glDeleteFramebuffers(GLsizei n, const GLuint* framebuffers);
    void glBindFramebuffer(GLenum target, GLuint framebuffer);
    void glFramebufferTexture2D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
    void glFramebufferRenderbuffer(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
    GLenum glCheckFramebufferStatus(GLenum target);
    void glDrawBuffers(GLsizei n, const GLenum* bufs);

    // Renderbuffer
    void glGenRenderbuffers(GLsizei n, GLuint* renderbuffers);
    void glDeleteRenderbuffers(GLsizei n, const GLuint* renderbuffers);
    void glBindRenderbuffer(GLenum target, GLuint renderbuffer);
    void glRenderbufferStorage(GLenum target, GLenum internalformat, GLsizei width, GLsizei height);

    // Query / Debug
    GLenum glGetError();
    const GLubyte* glGetString(GLenum name);
    const GLubyte* glGetStringi(GLenum name, GLuint index);
    void glGetIntegerv(GLenum pname, GLint* data);
    void glGetFloatv(GLenum pname, GLfloat* data);
    void glGetBooleanv(GLenum pname, GLboolean* data);
    void glFinish();
    void glFlush();

    typedef void (*GLDEBUGPROC)(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam);
    void glDebugMessageCallback(GLDEBUGPROC callback, const void* userParam);
}
