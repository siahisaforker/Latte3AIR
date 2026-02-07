// GL compatibility layer for Wii U — software rasterizer + GX2 fast-path
// This file implements a broad subset of GL entrypoints used by the engine.

#include "gl_compat.h"
#include "WiiUGfx.h"

#include "../rmxmedia/framework/wiiu_shim_gx2.h"
#include "../rmxmedia/framework/GX2Renderer.h"

#include "rmxbase/tools/Logging.h"

#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <cstring>
#include <mutex>
#include <string>
#include <array>
#include <cmath>
#include <algorithm>
#include <cctype>
#include <set>
#include <numeric>

// ============================================================================
// Internal state
// ============================================================================
namespace {

    // ----- Texture tracking -----
    struct TextureData {
        int w = 0, h = 0;
        std::vector<uint32_t> pixels;
        int minFilter = GL_NEAREST;
        int magFilter = GL_NEAREST;
        int wrapS = GL_CLAMP_TO_EDGE;
        int wrapT = GL_CLAMP_TO_EDGE;
        int whbHandle = 0; // WHB/GX2 texture handle when fast-path is active
    };

    std::unordered_map<GLuint, TextureData> g_textures;
    GLuint g_nextTexture = 1;
    GLuint g_boundTexture = 0;
    GLuint g_boundTextures[16] = {0};
    int g_activeTextureUnit = 0;

    // ----- Buffer tracking -----
    std::unordered_map<GLuint, std::vector<uint8_t>> g_buffers;
    GLuint g_nextBuffer = 1;
    GLuint g_boundArrayBuffer = 0;
    GLuint g_boundElementArrayBuffer = 0;
    GLuint g_boundTextureBuffer = 0;
    GLuint g_boundUniformBuffer = 0;
    GLuint g_currentProgram = 0;

    // ----- Shader / program bookkeeping -----
    struct ShaderInfo {
        GLenum type = 0;
        std::string source;
        bool compiled = false;
    };
    struct ProgramInfo {
        std::vector<GLuint> attachedShaders;
        std::unordered_map<std::string, int> uniformLocations;
        std::unordered_map<std::string, int> attribLocations;
        std::unordered_map<int, std::vector<int>> uniformInts;
        std::unordered_map<int, std::vector<float>> uniformFloats;
        int nextUniformLocation = 1;
        int nextAttribLocation = 0;
        bool linked = false;
    };
    std::unordered_map<GLuint, ShaderInfo> g_shaders;
    std::unordered_map<GLuint, ProgramInfo> g_programs;
    GLuint g_nextShader = 1;
    GLuint g_nextProgram = 1;

    // ----- Vertex attrib state -----
    struct VertexAttribState {
        bool enabled = false;
        GLint size = 0;
        GLenum type = 0;
        bool normalized = false;
        GLsizei stride = 0;
        const void* pointer = nullptr;
        size_t bufferOffset = 0;
        bool usesBuffer = false;
        GLuint divisor = 0;
    };
    static constexpr int MAX_ATTRIBS = 16;
    VertexAttribState g_attribs[MAX_ATTRIBS];

    // ----- VAO support -----
    std::unordered_map<GLuint, std::array<VertexAttribState, MAX_ATTRIBS>> g_vertexArrays;
    GLuint g_currentVAO = 0;
    GLuint g_nextVAO = 1;

    // ----- Buffer-range bindings -----
    struct BufferRange { GLuint buffer = 0; size_t offset = 0; size_t size = 0; };
    std::unordered_map<GLuint, BufferRange> g_bufferRanges;

    // ----- Framebuffer tracking -----
    struct FramebufferInfo {
        std::unordered_map<GLenum, GLuint> attachments; // attachment -> texture
    };
    std::unordered_map<GLuint, FramebufferInfo> g_framebuffers;
    GLuint g_nextFramebuffer = 1;
    GLuint g_boundFramebuffer = 0;

    // ----- Renderbuffer tracking -----
    std::unordered_set<GLuint> g_renderbuffers;
    GLuint g_nextRenderbuffer = 1;
    GLuint g_boundRenderbuffer = 0;

    // ----- State tracking -----
    std::unordered_set<GLenum> g_enabledCaps;

    struct BlendState {
        GLenum srcRGB = GL_ONE, dstRGB = GL_ZERO;
        GLenum srcAlpha = GL_ONE, dstAlpha = GL_ZERO;
        GLenum eqRGB = GL_FUNC_ADD, eqAlpha = GL_FUNC_ADD;
    } g_blend;

    struct ViewportState { GLint x = 0, y = 0; GLsizei w = 0, h = 0; } g_viewport;
    struct ScissorState { GLint x = 0, y = 0; GLsizei w = 0, h = 0; } g_scissor;

    // ----- Backbuffer -----
    std::vector<uint32_t> g_backbuffer;
    int g_width = 0, g_height = 0;
    std::mutex g_mutex;

    GLfloat g_clearColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};

    // ----- Pixel store state -----
    GLint g_unpackAlignment = 4;
    GLint g_packAlignment = 4;
    GLint g_unpackRowLength = 0;

    // ----- Helper: get bound buffer id for target -----
    GLuint getBoundBuffer(GLenum target) {
        switch (target) {
            case GL_ARRAY_BUFFER: return g_boundArrayBuffer;
            case GL_ELEMENT_ARRAY_BUFFER: return g_boundElementArrayBuffer;
            case GL_TEXTURE_BUFFER: return g_boundTextureBuffer;
            case GL_UNIFORM_BUFFER: return g_boundUniformBuffer;
            default: return 0;
        }
    }
}

// ============================================================================
// extern "C" implementations
// ============================================================================
extern "C" {

// ---- Lifecycle ----
void wiiu_gl_initialize(int width, int height)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    g_width = width; g_height = height;
    g_backbuffer.assign(static_cast<size_t>(width) * static_cast<size_t>(height), 0);
    g_viewport = {0, 0, static_cast<GLsizei>(width), static_cast<GLsizei>(height)};
    g_scissor = {0, 0, static_cast<GLsizei>(width), static_cast<GLsizei>(height)};
    if (rmx::WiiUGfx::isGX2Active())
    {
        rmx::GX2Renderer::instance().initialize(width, height);
    }
}

void wiiu_gl_shutdown()
{
    std::lock_guard<std::mutex> lock(g_mutex);
    for (auto& kv : g_textures) {
        if (kv.second.whbHandle != 0) {
            WHBGfxDestroyTexture(kv.second.whbHandle);
            kv.second.whbHandle = 0;
        }
    }
    g_textures.clear();
    g_buffers.clear();
    g_shaders.clear();
    g_programs.clear();
    g_vertexArrays.clear();
    g_framebuffers.clear();
    g_renderbuffers.clear();
    g_backbuffer.clear();
    if (rmx::WiiUGfx::isGX2Active()) rmx::GX2Renderer::instance().shutdown();
}

void wiiu_gl_present()
{
    std::lock_guard<std::mutex> lock(g_mutex);
    if (g_backbuffer.empty()) return;
    rmx::WiiUGfx::present(g_backbuffer.data(), g_width, g_height);
}

// ---- Textures ----
void glGenTextures(GLsizei n, GLuint* textures)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    for (GLsizei i = 0; i < n; ++i)
        textures[i] = g_nextTexture++;
}

void glDeleteTextures(GLsizei n, const GLuint* textures)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    for (GLsizei i = 0; i < n; ++i) {
        auto it = g_textures.find(textures[i]);
        if (it != g_textures.end()) {
            if (it->second.whbHandle != 0)
                WHBGfxDestroyTexture(it->second.whbHandle);
            g_textures.erase(it);
        }
    }
}

void glBindTexture(GLenum /*target*/, GLuint texture)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    g_boundTexture = texture;
    if (g_activeTextureUnit >= 0 && g_activeTextureUnit < 16)
        g_boundTextures[g_activeTextureUnit] = texture;
    if (rmx::WiiUGfx::isGX2Active() && texture != 0) {
        auto it = g_textures.find(texture);
        if (it != g_textures.end() && it->second.whbHandle != 0)
            WHBGfxBindTexture(it->second.whbHandle);
        else
            WHBGfxUnbindTexture();
    }
}

void glActiveTexture(GLenum texture)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    int unit = static_cast<int>(texture - GL_TEXTURE0);
    if (unit >= 0 && unit < 16) g_activeTextureUnit = unit;
}

static void uploadRGBAtoTex(TextureData& t, const void* pixels, int width, int height)
{
    t.w = width; t.h = height;
    t.pixels.resize(static_cast<size_t>(width) * static_cast<size_t>(height));
    if (!pixels) return;
    const uint8_t* src = static_cast<const uint8_t*>(pixels);
    for (int i = 0; i < width * height; ++i) {
        uint8_t r = src[i * 4 + 0], g = src[i * 4 + 1], b = src[i * 4 + 2], a = src[i * 4 + 3];
        t.pixels[i] = (uint32_t(a) << 24) | (uint32_t(b) << 16) | (uint32_t(g) << 8) | uint32_t(r);
    }
}

void glTexImage2D(GLenum, GLint, GLint, GLsizei width, GLsizei height, GLint, GLenum format, GLenum type, const void* pixels)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    if (g_boundTexture == 0) return;
    if (width <= 0 || height <= 0 || width > 8192 || height > 8192) return;

    TextureData& t = g_textures[g_boundTexture];

    if (format == GL_RGBA && type == GL_UNSIGNED_BYTE) {
        uploadRGBAtoTex(t, pixels, width, height);
        // Fast-path: create WHB/GX2 texture
        if (rmx::WiiUGfx::isGX2Active()) {
            if (t.whbHandle != 0) { WHBGfxDestroyTexture(t.whbHandle); t.whbHandle = 0; }
            if (pixels) {
                int h = WHBGfxCreateTexture(width, height, 0/*RGBA8*/, pixels);
                if (h >= 0) t.whbHandle = h;
            }
        }
    } else if (format == GL_RGB && type == GL_UNSIGNED_BYTE) {
        t.w = width; t.h = height;
        t.pixels.resize(static_cast<size_t>(width) * height);
        if (pixels) {
            const uint8_t* src = static_cast<const uint8_t*>(pixels);
            for (int i = 0; i < width * height; ++i) {
                t.pixels[i] = (0xFFu << 24) | (uint32_t(src[i*3+2]) << 16) | (uint32_t(src[i*3+1]) << 8) | uint32_t(src[i*3+0]);
            }
        }
    } else if (format == GL_LUMINANCE && type == GL_UNSIGNED_BYTE) {
        t.w = width; t.h = height;
        t.pixels.resize(static_cast<size_t>(width) * height);
        if (pixels) {
            const uint8_t* src = static_cast<const uint8_t*>(pixels);
            for (int i = 0; i < width * height; ++i) {
                uint8_t v = src[i];
                t.pixels[i] = (0xFFu << 24) | (uint32_t(v) << 16) | (uint32_t(v) << 8) | uint32_t(v);
            }
        }
    } else if ((format == GL_RED || format == GL_ALPHA) && type == GL_UNSIGNED_BYTE) {
        t.w = width; t.h = height;
        t.pixels.resize(static_cast<size_t>(width) * height);
        if (pixels) {
            const uint8_t* src = static_cast<const uint8_t*>(pixels);
            for (int i = 0; i < width * height; ++i) {
                uint8_t v = src[i];
                t.pixels[i] = (0xFFu << 24) | (uint32_t(v) << 16) | (uint32_t(v) << 8) | uint32_t(v);
            }
        }
    } else {
        t.w = width; t.h = height;
        t.pixels.resize(static_cast<size_t>(width) * height, 0);
    }
}

void glTexSubImage2D(GLenum, GLint, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void* pixels)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    if (g_boundTexture == 0) return;
    auto it = g_textures.find(g_boundTexture);
    if (it == g_textures.end()) return;
    TextureData& t = it->second;
    if (t.pixels.empty() || !pixels) return;
    if (xoffset < 0 || yoffset < 0 || xoffset + width > t.w || yoffset + height > t.h) return;

    if (format == GL_RGBA && type == GL_UNSIGNED_BYTE) {
        const uint8_t* src = static_cast<const uint8_t*>(pixels);
        for (int row = 0; row < height; ++row) {
            int dstY = yoffset + row;
            for (int col = 0; col < width; ++col) {
                int dstX = xoffset + col;
                size_t sidx = static_cast<size_t>(row * width + col) * 4;
                t.pixels[dstY * t.w + dstX] = (uint32_t(src[sidx+3]) << 24) | (uint32_t(src[sidx+2]) << 16) |
                                               (uint32_t(src[sidx+1]) << 8) | uint32_t(src[sidx+0]);
            }
        }
        // Update WHB texture if active
        if (rmx::WiiUGfx::isGX2Active() && t.whbHandle != 0)
            WHBGfxUpdateTexture(t.whbHandle, xoffset, yoffset, width, height, pixels);
    } else if (format == GL_LUMINANCE && type == GL_UNSIGNED_BYTE) {
        const uint8_t* src = static_cast<const uint8_t*>(pixels);
        for (int row = 0; row < height; ++row) {
            int dstY = yoffset + row;
            for (int col = 0; col < width; ++col) {
                int dstX = xoffset + col;
                uint8_t v = src[row * width + col];
                t.pixels[dstY * t.w + dstX] = (0xFFu << 24) | (uint32_t(v) << 16) | (uint32_t(v) << 8) | uint32_t(v);
            }
        }
    }
}

void glTexParameteri(GLenum, GLenum pname, GLint param)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    if (g_boundTexture == 0) return;
    auto it = g_textures.find(g_boundTexture);
    if (it == g_textures.end()) return;
    TextureData& t = it->second;
    if (pname == GL_TEXTURE_MIN_FILTER) t.minFilter = param;
    else if (pname == GL_TEXTURE_MAG_FILTER) t.magFilter = param;
    else if (pname == GL_TEXTURE_WRAP_S) t.wrapS = param;
    else if (pname == GL_TEXTURE_WRAP_T) t.wrapT = param;
}

void glTexParameterf(GLenum target, GLenum pname, GLfloat param) { glTexParameteri(target, pname, static_cast<GLint>(param)); }
GLboolean glIsTexture(GLuint texture) { std::lock_guard<std::mutex> lock(g_mutex); return g_textures.count(texture) ? GL_TRUE : GL_FALSE; }
void glCopyTexImage2D(GLenum, GLint, GLenum, GLint, GLint, GLsizei, GLsizei, GLint) {}
void glCopyTexSubImage2D(GLenum, GLint, GLint, GLint, GLint, GLint, GLsizei, GLsizei) {}
void glGenerateMipmap(GLenum) {}
void glPixelStorei(GLenum pname, GLint param) {
    if (pname == GL_UNPACK_ALIGNMENT) g_unpackAlignment = param;
    else if (pname == GL_PACK_ALIGNMENT) g_packAlignment = param;
    else if (pname == GL_UNPACK_ROW_LENGTH) g_unpackRowLength = param;
}

void glReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void* pixels)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    if (!pixels || g_backbuffer.empty()) return;
    if (format == GL_RGBA && type == GL_UNSIGNED_BYTE) {
        uint8_t* dst = static_cast<uint8_t*>(pixels);
        for (int row = 0; row < height; ++row) {
            int srcY = y + row;
            if (srcY < 0 || srcY >= g_height) continue;
            for (int col = 0; col < width; ++col) {
                int srcX = x + col;
                if (srcX < 0 || srcX >= g_width) continue;
                uint32_t c = g_backbuffer[srcY * g_width + srcX];
                size_t di = static_cast<size_t>(row * width + col) * 4;
                dst[di + 0] = c & 0xFF;
                dst[di + 1] = (c >> 8) & 0xFF;
                dst[di + 2] = (c >> 16) & 0xFF;
                dst[di + 3] = (c >> 24) & 0xFF;
            }
        }
    }
}

void glTexBuffer(GLenum target, GLenum, GLuint buffer)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    if (target != GL_TEXTURE_BUFFER || g_boundTexture == 0) return;
    auto bit = g_buffers.find(buffer);
    if (bit == g_buffers.end()) return;
    auto& t = g_textures[g_boundTexture];
    const auto& buf = bit->second;
    size_t width = buf.empty() ? 1 : buf.size();
    t.w = static_cast<int>(width); t.h = 1;
    t.pixels.resize(width);
    for (size_t i = 0; i < buf.size(); ++i) {
        uint8_t v = buf[i];
        t.pixels[i] = (0xFFu << 24) | (uint32_t(v) << 16) | (uint32_t(v) << 8) | uint32_t(v);
    }
}

void glTexBufferRange(GLenum target, GLint, GLuint buffer, GLintptr offset, GLsizeiptr size)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    if (target != GL_TEXTURE_BUFFER || g_boundTexture == 0) return;
    auto bit = g_buffers.find(buffer);
    if (bit == g_buffers.end()) return;
    auto& t = g_textures[g_boundTexture];
    const auto& buf = bit->second;
    size_t off = std::min(static_cast<size_t>(offset), buf.size());
    size_t sz = std::min(static_cast<size_t>(size), buf.size() - off);
    size_t width = sz == 0 ? 1 : sz;
    t.w = static_cast<int>(width); t.h = 1;
    t.pixels.resize(width);
    for (size_t i = 0; i < sz; ++i) {
        uint8_t v = buf[off + i];
        t.pixels[i] = (0xFFu << 24) | (uint32_t(v) << 16) | (uint32_t(v) << 8) | uint32_t(v);
    }
}

// ---- Buffers ----
void glGenBuffers(GLsizei n, GLuint* buffers)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    for (GLsizei i = 0; i < n; ++i) buffers[i] = g_nextBuffer++;
}

void glDeleteBuffers(GLsizei n, const GLuint* buffers)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    for (GLsizei i = 0; i < n; ++i) g_buffers.erase(buffers[i]);
}

void glBindBuffer(GLenum target, GLuint buffer)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    switch (target) {
        case GL_ARRAY_BUFFER: g_boundArrayBuffer = buffer; break;
        case GL_ELEMENT_ARRAY_BUFFER: g_boundElementArrayBuffer = buffer; break;
        case GL_TEXTURE_BUFFER: g_boundTextureBuffer = buffer; break;
        case GL_UNIFORM_BUFFER: g_boundUniformBuffer = buffer; break;
        default: break;
    }
}

void glBufferData(GLenum target, GLsizeiptr size, const void* data, GLenum)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    GLuint id = getBoundBuffer(target);
    if (id == 0) return;
    auto& buf = g_buffers[id];
    buf.resize(static_cast<size_t>(size));
    if (data && size > 0) std::memcpy(buf.data(), data, static_cast<size_t>(size));
}

void glBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, const void* data)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    GLuint id = getBoundBuffer(target);
    if (id == 0 || !data) return;
    auto it = g_buffers.find(id);
    if (it == g_buffers.end()) return;
    size_t off = static_cast<size_t>(offset);
    size_t sz = static_cast<size_t>(size);
    if (off + sz > it->second.size()) return;
    std::memcpy(it->second.data() + off, data, sz);
}

void* glMapBuffer(GLenum target, GLenum)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    GLuint id = getBoundBuffer(target);
    if (id == 0) return nullptr;
    auto it = g_buffers.find(id);
    return (it != g_buffers.end()) ? it->second.data() : nullptr;
}

void* glMapBufferRange(GLenum target, GLintptr offset, GLsizeiptr length, GLuint)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    GLuint id = getBoundBuffer(target);
    if (id == 0) return nullptr;
    auto it = g_buffers.find(id);
    if (it == g_buffers.end()) return nullptr;
    size_t off = static_cast<size_t>(offset);
    if (off >= it->second.size()) return nullptr;
    return it->second.data() + off;
}

GLboolean glUnmapBuffer(GLenum) { return GL_TRUE; }

void glBindBufferRange(GLenum, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    g_bufferRanges[index] = {buffer, static_cast<size_t>(offset), static_cast<size_t>(size)};
}

void glBindBufferBase(GLenum target, GLuint index, GLuint buffer)
{
    glBindBufferRange(target, index, buffer, 0, 0);
}

// ---- VAO ----
void glGenVertexArrays(GLsizei n, GLuint* arrays)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    for (GLsizei i = 0; i < n; ++i) {
        GLuint id = g_nextVAO++;
        arrays[i] = id;
        g_vertexArrays[id] = {};
    }
}

void glBindVertexArray(GLuint array)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    g_currentVAO = array;
    if (array == 0) return;
    auto it = g_vertexArrays.find(array);
    if (it == g_vertexArrays.end())
        g_vertexArrays[array] = {};
    for (int i = 0; i < MAX_ATTRIBS; ++i)
        g_attribs[i] = g_vertexArrays[array][i];
}

void glDeleteVertexArrays(GLsizei n, const GLuint* arrays)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    for (GLsizei i = 0; i < n; ++i) g_vertexArrays.erase(arrays[i]);
}

// ---- Vertex attribs ----
void glVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* pointer)
{
    if (index >= MAX_ATTRIBS) return;
    auto* a = (g_currentVAO != 0) ? &g_vertexArrays[g_currentVAO][index] : &g_attribs[index];
    a->size = size; a->type = type; a->normalized = (normalized != GL_FALSE); a->stride = stride;
    if (g_boundArrayBuffer != 0) {
        a->usesBuffer = true;
        a->bufferOffset = reinterpret_cast<size_t>(pointer);
        a->pointer = nullptr;
    } else {
        a->usesBuffer = false;
        a->pointer = pointer;
        a->bufferOffset = 0;
    }
}

void glVertexAttribIPointer(GLuint index, GLint size, GLenum type, GLsizei stride, const void* pointer)
{
    glVertexAttribPointer(index, size, type, GL_FALSE, stride, pointer);
}

void glVertexAttribDivisor(GLuint index, GLuint divisor)
{
    if (index >= MAX_ATTRIBS) return;
    if (g_currentVAO != 0) g_vertexArrays[g_currentVAO][index].divisor = divisor;
    else g_attribs[index].divisor = divisor;
}

void glEnableVertexAttribArray(GLuint index)
{
    if (index >= MAX_ATTRIBS) return;
    if (g_currentVAO != 0) g_vertexArrays[g_currentVAO][index].enabled = true;
    else g_attribs[index].enabled = true;
}

void glDisableVertexAttribArray(GLuint index)
{
    if (index >= MAX_ATTRIBS) return;
    if (g_currentVAO != 0) g_vertexArrays[g_currentVAO][index].enabled = false;
    else g_attribs[index].enabled = false;
}

// ---- Draw helpers ----
static inline std::pair<int,int> ndcToScreen(float nx, float ny, int width, int height)
{
    float sx = (nx * 0.5f + 0.5f) * static_cast<float>(width);
    float sy = (1.0f - (ny * 0.5f + 0.5f)) * static_cast<float>(height);
    return {static_cast<int>(std::floor(sx)), static_cast<int>(std::floor(sy))};
}

static inline const float* readAttribFloat(const VertexAttribState& a, uint32_t idx)
{
    if (!a.enabled) return nullptr;
    if (a.usesBuffer && g_boundArrayBuffer != 0) {
        auto it = g_buffers.find(g_boundArrayBuffer);
        if (it != g_buffers.end()) {
            size_t elemSize = a.stride ? a.stride : (a.size * sizeof(float));
            size_t base = a.bufferOffset + static_cast<size_t>(idx) * elemSize;
            if (base + a.size * sizeof(float) <= it->second.size())
                return reinterpret_cast<const float*>(it->second.data() + base);
        }
    } else if (a.pointer) {
        const char* base = reinterpret_cast<const char*>(a.pointer);
        size_t elemSize = a.stride ? a.stride : (a.size * sizeof(float));
        return reinterpret_cast<const float*>(base + static_cast<size_t>(idx) * elemSize);
    }
    return nullptr;
}

struct SoftVert { float x = 0, y = 0, u = 0, v = 0; };

static void buildVerts(const uint32_t* indices, size_t count, std::vector<SoftVert>& verts)
{
    verts.resize(count);
    for (size_t i = 0; i < count; ++i) {
        uint32_t idx = indices[i];
        SoftVert& v = verts[i];
        const float* pos = readAttribFloat(g_attribs[0], idx);
        if (pos) { v.x = pos[0]; v.y = pos[1]; }
        const float* uv = readAttribFloat(g_attribs[1], idx);
        if (uv) { v.u = uv[0]; v.v = uv[1]; }
    }
}

static uint32_t sampleTex(const TextureData& t, float u, float v)
{
    if (t.pixels.empty()) return 0;
    int tx = static_cast<int>(u * (t.w - 1) + 0.5f);
    int ty = static_cast<int>(v * (t.h - 1) + 0.5f);
    tx = std::clamp(tx, 0, t.w - 1);
    ty = std::clamp(ty, 0, t.h - 1);
    return t.pixels[ty * t.w + tx];
}

static void softwareRasterize(const std::vector<SoftVert>& verts, int gw, int gh, std::vector<uint32_t>& bb,
                               const TextureData* tex)
{
    for (size_t tri = 0; tri + 2 < verts.size(); tri += 3) {
        const SoftVert& v0 = verts[tri], &v1 = verts[tri+1], &v2 = verts[tri+2];
        auto p0 = ndcToScreen(v0.x, v0.y, gw, gh);
        auto p1 = ndcToScreen(v1.x, v1.y, gw, gh);
        auto p2 = ndcToScreen(v2.x, v2.y, gw, gh);

        int minx = std::max(0, std::min({p0.first, p1.first, p2.first}));
        int maxx = std::min(gw - 1, std::max({p0.first, p1.first, p2.first}));
        int miny = std::max(0, std::min({p0.second, p1.second, p2.second}));
        int maxy = std::min(gh - 1, std::max({p0.second, p1.second, p2.second}));

        float x0f = (float)p0.first, y0f = (float)p0.second;
        float x1f = (float)p1.first, y1f = (float)p1.second;
        float x2f = (float)p2.first, y2f = (float)p2.second;
        float denom = (y1f - y2f) * (x0f - x2f) + (x2f - x1f) * (y0f - y2f);
        if (std::fabs(denom) < 1e-6f) continue;

        for (int y = miny; y <= maxy; ++y) {
            for (int x = minx; x <= maxx; ++x) {
                float px = (float)x, py = (float)y;
                float w0 = ((y1f - y2f) * (px - x2f) + (x2f - x1f) * (py - y2f)) / denom;
                float w1 = ((y2f - y0f) * (px - x2f) + (x0f - x2f) * (py - y2f)) / denom;
                float w2 = 1.0f - w0 - w1;
                if (w0 >= 0 && w1 >= 0 && w2 >= 0) {
                    float tu = w0 * v0.u + w1 * v1.u + w2 * v2.u;
                    float tv = w0 * v0.v + w1 * v1.v + w2 * v2.v;
                    uint32_t color = tex ? sampleTex(*tex, tu, tv) : 0xFFFFFFFFu;
                    bb[y * gw + x] = color;
                }
            }
        }
    }
}

void glDrawArrays(GLenum, GLint first, GLsizei count)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    if (g_backbuffer.empty() || count <= 0) return;

    std::vector<uint32_t> indices(count);
    std::iota(indices.begin(), indices.end(), static_cast<uint32_t>(first));

    std::vector<SoftVert> verts;
    buildVerts(indices.data(), indices.size(), verts);

    // GX2 fast-path
    if (rmx::WiiUGfx::isGX2Active() && g_boundTexture != 0) {
        auto itTex = g_textures.find(g_boundTexture);
        if (itTex != g_textures.end() && itTex->second.whbHandle != 0) {
            for (size_t tri = 0; tri + 2 < verts.size(); tri += 3) {
                auto p0 = ndcToScreen(verts[tri].x, verts[tri].y, g_width, g_height);
                auto p1 = ndcToScreen(verts[tri+1].x, verts[tri+1].y, g_width, g_height);
                auto p2 = ndcToScreen(verts[tri+2].x, verts[tri+2].y, g_width, g_height);
                WHBGfxDrawTexturedTriangle(p0.first, p0.second, verts[tri].u, verts[tri].v,
                                            p1.first, p1.second, verts[tri+1].u, verts[tri+1].v,
                                            p2.first, p2.second, verts[tri+2].u, verts[tri+2].v);
            }
            return;
        }
    }

    const TextureData* tex = nullptr;
    if (g_boundTexture != 0) {
        auto it = g_textures.find(g_boundTexture);
        if (it != g_textures.end()) tex = &it->second;
    }
    softwareRasterize(verts, g_width, g_height, g_backbuffer, tex);
}

void glDrawArraysInstanced(GLenum mode, GLint first, GLsizei count, GLsizei instancecount)
{
    for (GLsizei i = 0; i < instancecount; ++i)
        glDrawArrays(mode, first, count);
}

static void gatherIndices(GLsizei count, GLenum type, const void* indices, std::vector<uint32_t>& out)
{
    out.reserve(count);
    if (g_boundElementArrayBuffer != 0) {
        auto it = g_buffers.find(g_boundElementArrayBuffer);
        if (it == g_buffers.end()) return;
        const auto& buf = it->second;
        size_t byteOff = reinterpret_cast<size_t>(indices);
        if (type == GL_UNSIGNED_SHORT) {
            for (GLsizei i = 0; i < count; ++i) {
                size_t off = byteOff + i * 2;
                if (off + 2 > buf.size()) break;
                out.push_back(*reinterpret_cast<const uint16_t*>(buf.data() + off));
            }
        } else if (type == GL_UNSIGNED_INT) {
            for (GLsizei i = 0; i < count; ++i) {
                size_t off = byteOff + i * 4;
                if (off + 4 > buf.size()) break;
                out.push_back(*reinterpret_cast<const uint32_t*>(buf.data() + off));
            }
        } else {
            for (GLsizei i = 0; i < count; ++i) {
                size_t off = byteOff + i;
                if (off >= buf.size()) break;
                out.push_back(buf[off]);
            }
        }
    } else if (indices) {
        if (type == GL_UNSIGNED_SHORT) {
            const uint16_t* d = static_cast<const uint16_t*>(indices);
            for (GLsizei i = 0; i < count; ++i) out.push_back(d[i]);
        } else if (type == GL_UNSIGNED_INT) {
            const uint32_t* d = static_cast<const uint32_t*>(indices);
            for (GLsizei i = 0; i < count; ++i) out.push_back(d[i]);
        } else {
            const uint8_t* d = static_cast<const uint8_t*>(indices);
            for (GLsizei i = 0; i < count; ++i) out.push_back(d[i]);
        }
    }
}

void glDrawElements(GLenum, GLsizei count, GLenum type, const void* indices)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    if (g_backbuffer.empty() || count <= 0) return;

    std::vector<uint32_t> idxs;
    gatherIndices(count, type, indices, idxs);
    if (idxs.empty()) return;

    std::vector<SoftVert> verts;
    buildVerts(idxs.data(), idxs.size(), verts);

    // GX2 fast-path
    if (rmx::WiiUGfx::isGX2Active() && g_boundTexture != 0) {
        auto itTex = g_textures.find(g_boundTexture);
        if (itTex != g_textures.end() && itTex->second.whbHandle != 0) {
            for (size_t tri = 0; tri + 2 < verts.size(); tri += 3) {
                auto p0 = ndcToScreen(verts[tri].x, verts[tri].y, g_width, g_height);
                auto p1 = ndcToScreen(verts[tri+1].x, verts[tri+1].y, g_width, g_height);
                auto p2 = ndcToScreen(verts[tri+2].x, verts[tri+2].y, g_width, g_height);
                WHBGfxDrawTexturedTriangle(p0.first, p0.second, verts[tri].u, verts[tri].v,
                                            p1.first, p1.second, verts[tri+1].u, verts[tri+1].v,
                                            p2.first, p2.second, verts[tri+2].u, verts[tri+2].v);
            }
            return;
        }
    }

    const TextureData* tex = nullptr;
    if (g_boundTexture != 0) {
        auto it = g_textures.find(g_boundTexture);
        if (it != g_textures.end()) tex = &it->second;
    }
    softwareRasterize(verts, g_width, g_height, g_backbuffer, tex);
}

void glDrawElementsInstanced(GLenum mode, GLsizei count, GLenum type, const void* indices, GLsizei instancecount)
{
    for (GLsizei i = 0; i < instancecount; ++i)
        glDrawElements(mode, count, type, indices);
}

void glDrawRangeElements(GLenum mode, GLuint, GLuint, GLsizei count, GLenum type, const void* indices)
{
    glDrawElements(mode, count, type, indices);
}

// ---- State ----
void glEnable(GLenum cap) { g_enabledCaps.insert(cap); }
void glDisable(GLenum cap) { g_enabledCaps.erase(cap); }
GLboolean glIsEnabled(GLenum cap) { return g_enabledCaps.count(cap) ? GL_TRUE : GL_FALSE; }

void glBlendFunc(GLenum sf, GLenum df) { g_blend.srcRGB = sf; g_blend.dstRGB = df; g_blend.srcAlpha = sf; g_blend.dstAlpha = df; }
void glBlendFuncSeparate(GLenum sR, GLenum dR, GLenum sA, GLenum dA) { g_blend.srcRGB = sR; g_blend.dstRGB = dR; g_blend.srcAlpha = sA; g_blend.dstAlpha = dA; }
void glBlendEquation(GLenum m) { g_blend.eqRGB = m; g_blend.eqAlpha = m; }
void glBlendEquationSeparate(GLenum mR, GLenum mA) { g_blend.eqRGB = mR; g_blend.eqAlpha = mA; }
void glDepthFunc(GLenum) {}
void glDepthMask(GLboolean) {}
void glColorMask(GLboolean, GLboolean, GLboolean, GLboolean) {}
void glCullFace(GLenum) {}
void glFrontFace(GLenum) {}
void glLineWidth(GLfloat) {}
void glPointSize(GLfloat) {}
void glPolygonOffset(GLfloat, GLfloat) {}

void glScissor(GLint x, GLint y, GLsizei w, GLsizei h)
{
    g_scissor = {x, y, w, h};
    if (rmx::WiiUGfx::isGX2Active())
        rmx::GX2Renderer::instance().setScissor(x, y, w, h);
}

void glViewport(GLint x, GLint y, GLsizei w, GLsizei h) { g_viewport = {x, y, w, h}; }

void glClearColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a) { g_clearColor[0] = r; g_clearColor[1] = g; g_clearColor[2] = b; g_clearColor[3] = a; }

void glClear(GLenum mask)
{
    if (mask & GL_COLOR_BUFFER_BIT) {
        uint32_t c = (uint32_t(g_clearColor[3]*255.f) << 24) | (uint32_t(g_clearColor[2]*255.f) << 16) |
                     (uint32_t(g_clearColor[1]*255.f) << 8)  | uint32_t(g_clearColor[0]*255.f);
        std::fill(g_backbuffer.begin(), g_backbuffer.end(), c);
    }
}

void glClearDepth(GLdouble) {}

// ---- Shader/Program ----
GLuint glCreateShader(GLenum type)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    GLuint id = g_nextShader++;
    g_shaders[id] = {type, "", false};
    return id;
}

void glDeleteShader(GLuint shader)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    g_shaders.erase(shader);
}

void glShaderSource(GLuint shader, GLsizei count, const GLchar* const* string, const GLint* length)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    auto it = g_shaders.find(shader);
    if (it == g_shaders.end()) return;
    it->second.source.clear();
    for (GLsizei i = 0; i < count; ++i) {
        if (length && length[i] > 0) it->second.source.append(string[i], length[i]);
        else it->second.source.append(string[i]);
    }
}

void glCompileShader(GLuint shader)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    auto it = g_shaders.find(shader);
    if (it != g_shaders.end()) it->second.compiled = true;
}

void glGetShaderiv(GLuint shader, GLenum pname, GLint* params)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    if (!params) return;
    auto it = g_shaders.find(shader);
    if (it == g_shaders.end()) { *params = 0; return; }
    switch (pname) {
        case GL_COMPILE_STATUS: *params = it->second.compiled ? GL_TRUE : GL_FALSE; break;
        case GL_INFO_LOG_LENGTH: *params = 1; break;
        case GL_SHADER_TYPE: *params = it->second.type; break;
        case GL_DELETE_STATUS: *params = GL_FALSE; break;
        default: *params = 0;
    }
}

void glGetShaderInfoLog(GLuint, GLsizei bufSize, GLsizei* length, GLchar* infoLog)
{
    if (length) *length = 0;
    if (infoLog && bufSize > 0) infoLog[0] = '\0';
}

GLuint glCreateProgram()
{
    std::lock_guard<std::mutex> lock(g_mutex);
    GLuint id = g_nextProgram++;
    g_programs[id] = ProgramInfo();
    return id;
}

void glDeleteProgram(GLuint program)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    g_programs.erase(program);
}

void glAttachShader(GLuint program, GLuint shader)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    auto it = g_programs.find(program);
    if (it != g_programs.end()) it->second.attachedShaders.push_back(shader);
}

void glDetachShader(GLuint program, GLuint shader)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    auto it = g_programs.find(program);
    if (it == g_programs.end()) return;
    auto& v = it->second.attachedShaders;
    v.erase(std::remove(v.begin(), v.end(), shader), v.end());
}

void glLinkProgram(GLuint program)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    auto pit = g_programs.find(program);
    if (pit == g_programs.end()) return;
    ProgramInfo& info = pit->second;
    info.linked = true;

    // Parse attached shader sources for uniform/attribute declarations
    for (GLuint s : info.attachedShaders) {
        auto sit = g_shaders.find(s);
        if (sit == g_shaders.end()) continue;
        const std::string& src = sit->second.source;

        // Parse "uniform <type> <name>;" and "attribute/in <type> <name>;"
        for (const char* keyword : {"uniform", "attribute", "in"}) {
            size_t pos = 0;
            while (true) {
                pos = src.find(keyword, pos);
                if (pos == std::string::npos) break;
                size_t lineEnd = src.find(';', pos);
                if (lineEnd == std::string::npos) break;
                std::string decl = src.substr(pos, lineEnd - pos);
                // tokenize
                std::vector<std::string> words;
                size_t p = 0;
                while (p < decl.size()) {
                    while (p < decl.size() && std::isspace((unsigned char)decl[p])) ++p;
                    size_t q = p;
                    while (q < decl.size() && !std::isspace((unsigned char)decl[q])) ++q;
                    if (q > p) words.push_back(decl.substr(p, q - p));
                    p = q;
                }
                if (words.size() >= 3) {
                    std::string name = words[2];
                    size_t br = name.find('[');
                    if (br != std::string::npos) name = name.substr(0, br);
                    if (std::string(keyword) == "uniform") {
                        if (!info.uniformLocations.count(name))
                            info.uniformLocations[name] = info.nextUniformLocation++;
                    } else {
                        if (!info.attribLocations.count(name))
                            info.attribLocations[name] = info.nextAttribLocation++;
                    }
                }
                pos = lineEnd + 1;
            }
        }
    }
}

void glUseProgram(GLuint program) { std::lock_guard<std::mutex> lock(g_mutex); g_currentProgram = program; }
void glValidateProgram(GLuint) {}

void glGetProgramiv(GLuint program, GLenum pname, GLint* params)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    if (!params) return;
    auto it = g_programs.find(program);
    if (it == g_programs.end()) { *params = 0; return; }
    switch (pname) {
        case GL_LINK_STATUS: *params = it->second.linked ? GL_TRUE : GL_FALSE; break;
        case GL_INFO_LOG_LENGTH: *params = 1; break;
        case GL_ATTACHED_SHADERS: *params = static_cast<GLint>(it->second.attachedShaders.size()); break;
        case GL_ACTIVE_UNIFORMS: *params = static_cast<GLint>(it->second.uniformLocations.size()); break;
        case GL_ACTIVE_ATTRIBUTES: *params = static_cast<GLint>(it->second.attribLocations.size()); break;
        default: *params = 0;
    }
}

void glGetProgramInfoLog(GLuint, GLsizei bufSize, GLsizei* length, GLchar* infoLog)
{
    if (length) *length = 0;
    if (infoLog && bufSize > 0) infoLog[0] = '\0';
}

void glBindAttribLocation(GLuint program, GLuint index, const GLchar* name)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    auto it = g_programs.find(program);
    if (it == g_programs.end() || !name) return;
    it->second.attribLocations[name] = static_cast<int>(index);
}

GLint glGetUniformLocation(GLuint program, const GLchar* name)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    auto it = g_programs.find(program);
    if (it == g_programs.end() || !name) return -1;
    auto uit = it->second.uniformLocations.find(name);
    if (uit != it->second.uniformLocations.end()) return uit->second;
    // Auto-create location for convenience
    int loc = it->second.nextUniformLocation++;
    it->second.uniformLocations[name] = loc;
    return loc;
}

GLint glGetAttribLocation(GLuint program, const GLchar* name)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    auto it = g_programs.find(program);
    if (it == g_programs.end() || !name) return -1;
    auto ait = it->second.attribLocations.find(name);
    if (ait != it->second.attribLocations.end()) return ait->second;
    int loc = it->second.nextAttribLocation++;
    it->second.attribLocations[name] = loc;
    return loc;
}

// ---- Uniforms ----
#define UNIFORM_SET_I(loc, vals) do { \
    std::lock_guard<std::mutex> lock(g_mutex); \
    if (g_currentProgram == 0) return; \
    g_programs[g_currentProgram].uniformInts[loc] = vals; \
} while(0)

#define UNIFORM_SET_F(loc, vals) do { \
    std::lock_guard<std::mutex> lock(g_mutex); \
    if (g_currentProgram == 0) return; \
    g_programs[g_currentProgram].uniformFloats[loc] = vals; \
} while(0)

void glUniform1i(GLint loc, GLint v0)        { UNIFORM_SET_I(loc, (std::vector<int>{v0})); }
void glUniform2i(GLint loc, GLint a, GLint b) { UNIFORM_SET_I(loc, (std::vector<int>{a, b})); }
void glUniform3i(GLint loc, GLint a, GLint b, GLint c) { UNIFORM_SET_I(loc, (std::vector<int>{a, b, c})); }
void glUniform4i(GLint loc, GLint a, GLint b, GLint c, GLint d) { UNIFORM_SET_I(loc, (std::vector<int>{a, b, c, d})); }
void glUniform1iv(GLint loc, GLsizei count, const GLint* v) { UNIFORM_SET_I(loc, std::vector<int>(v, v + count)); }
void glUniform2iv(GLint loc, GLsizei count, const GLint* v) { UNIFORM_SET_I(loc, std::vector<int>(v, v + count*2)); }
void glUniform3iv(GLint loc, GLsizei count, const GLint* v) { UNIFORM_SET_I(loc, std::vector<int>(v, v + count*3)); }
void glUniform4iv(GLint loc, GLsizei count, const GLint* v) { UNIFORM_SET_I(loc, std::vector<int>(v, v + count*4)); }

void glUniform1f(GLint loc, GLfloat v0) { UNIFORM_SET_F(loc, (std::vector<float>{v0})); }
void glUniform2f(GLint loc, GLfloat a, GLfloat b) { UNIFORM_SET_F(loc, (std::vector<float>{a, b})); }
void glUniform3f(GLint loc, GLfloat a, GLfloat b, GLfloat c) { UNIFORM_SET_F(loc, (std::vector<float>{a, b, c})); }
void glUniform4f(GLint loc, GLfloat a, GLfloat b, GLfloat c, GLfloat d) { UNIFORM_SET_F(loc, (std::vector<float>{a, b, c, d})); }
void glUniform1fv(GLint loc, GLsizei count, const GLfloat* v) { UNIFORM_SET_F(loc, std::vector<float>(v, v + count)); }
void glUniform2fv(GLint loc, GLsizei count, const GLfloat* v) { UNIFORM_SET_F(loc, std::vector<float>(v, v + count*2)); }
void glUniform3fv(GLint loc, GLsizei count, const GLfloat* v) { UNIFORM_SET_F(loc, std::vector<float>(v, v + count*3)); }
void glUniform4fv(GLint loc, GLsizei count, const GLfloat* v) { UNIFORM_SET_F(loc, std::vector<float>(v, v + count*4)); }
void glUniformMatrix2fv(GLint loc, GLsizei count, GLboolean, const GLfloat* v) { UNIFORM_SET_F(loc, std::vector<float>(v, v + count*4)); }
void glUniformMatrix3fv(GLint loc, GLsizei count, GLboolean, const GLfloat* v) { UNIFORM_SET_F(loc, std::vector<float>(v, v + count*9)); }
void glUniformMatrix4fv(GLint loc, GLsizei count, GLboolean, const GLfloat* v) { UNIFORM_SET_F(loc, std::vector<float>(v, v + count*16)); }

// ---- Framebuffer ----
void glGenFramebuffers(GLsizei n, GLuint* fbs)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    for (GLsizei i = 0; i < n; ++i) { fbs[i] = g_nextFramebuffer++; g_framebuffers[fbs[i]] = {}; }
}

void glDeleteFramebuffers(GLsizei n, const GLuint* fbs)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    for (GLsizei i = 0; i < n; ++i) g_framebuffers.erase(fbs[i]);
}

void glBindFramebuffer(GLenum, GLuint fb) { std::lock_guard<std::mutex> lock(g_mutex); g_boundFramebuffer = fb; }

void glFramebufferTexture2D(GLenum, GLenum attachment, GLenum, GLuint texture, GLint)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    if (g_boundFramebuffer == 0) return;
    g_framebuffers[g_boundFramebuffer].attachments[attachment] = texture;
}

void glFramebufferRenderbuffer(GLenum, GLenum, GLenum, GLuint) {}

GLenum glCheckFramebufferStatus(GLenum) { return GL_FRAMEBUFFER_COMPLETE; }
void glDrawBuffers(GLsizei, const GLenum*) {}

// ---- Renderbuffer ----
void glGenRenderbuffers(GLsizei n, GLuint* rbs)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    for (GLsizei i = 0; i < n; ++i) { rbs[i] = g_nextRenderbuffer++; g_renderbuffers.insert(rbs[i]); }
}

void glDeleteRenderbuffers(GLsizei n, const GLuint* rbs)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    for (GLsizei i = 0; i < n; ++i) g_renderbuffers.erase(rbs[i]);
}

void glBindRenderbuffer(GLenum, GLuint rb) { g_boundRenderbuffer = rb; }
void glRenderbufferStorage(GLenum, GLenum, GLsizei, GLsizei) {}

// ---- Query / Debug ----
GLenum glGetError() { return GL_NO_ERROR; }

const GLubyte* glGetString(GLenum name)
{
    switch (name) {
        case GL_VENDOR:   return reinterpret_cast<const GLubyte*>("WiiU gl_compat");
        case GL_RENDERER: return reinterpret_cast<const GLubyte*>("WiiU Software/GX2");
        case GL_VERSION:  return reinterpret_cast<const GLubyte*>("2.1 WiiU compat");
        case GL_SHADING_LANGUAGE_VERSION: return reinterpret_cast<const GLubyte*>("1.10 WiiU compat");
        case GL_EXTENSIONS: return reinterpret_cast<const GLubyte*>("");
        default: return reinterpret_cast<const GLubyte*>("");
    }
}

const GLubyte* glGetStringi(GLenum, GLuint) { return reinterpret_cast<const GLubyte*>(""); }

void glGetIntegerv(GLenum pname, GLint* data)
{
    if (!data) return;
    switch (pname) {
        case GL_MAX_TEXTURE_SIZE: *data = 4096; break;
        case GL_MAJOR_VERSION: *data = 2; break;
        case GL_MINOR_VERSION: *data = 1; break;
        case GL_NUM_EXTENSIONS: *data = 0; break;
        case GL_VIEWPORT: data[0] = g_viewport.x; data[1] = g_viewport.y; data[2] = g_viewport.w; data[3] = g_viewport.h; break;
        default: *data = 0;
    }
}

void glGetFloatv(GLenum, GLfloat* data) { if (data) *data = 0; }
void glGetBooleanv(GLenum, GLboolean* data) { if (data) *data = GL_FALSE; }

void glFinish() {}
void glFlush() {}
void glDebugMessageCallback(GLDEBUGPROC, const void*) {}

} // extern "C"
