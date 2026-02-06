// Minimal, portable GL compatibility layer for Wii U
// This file implements a small subset of GL entrypoints used by the engine.

#include "gl_compat.h"
#include "WiiUGfx.h"

#include "../rmxmedia/framework/wiiu_shim_gx2.h"
#define GL_COMPAT_HAS_WHB 0

#include "../rmxmedia/framework/GX2Renderer.h"

#include "rmxbase/tools/Logging.h"

#include <vector>
#include <unordered_map>
#include <cstring>
#include <mutex>
#include <string>
#include <array>
#include <cmath>
#include <algorithm>
#include <cctype>

namespace {
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

    std::unordered_map<GLuint, std::vector<uint8_t>> g_buffers;
    GLuint g_nextBuffer = 1;
    GLuint g_boundArrayBuffer = 0;
    GLuint g_boundElementArrayBuffer = 0;
    GLuint g_boundTextureBuffer = 0; // bound buffer for GL_TEXTURE_BUFFER target
    GLuint g_currentProgram = 0;

    // Shader/program bookkeeping
    struct ProgramInfo {
        std::vector<GLuint> attachedShaders;
        std::unordered_map<std::string,int> uniformLocations;
        std::unordered_map<int, std::vector<int>> uniformInts;
        std::unordered_map<int, std::vector<float>> uniformFloats;
        int nextLocation = 1;
    };
    std::unordered_map<GLuint, std::string> g_shaderSources;
    std::unordered_map<GLuint, ProgramInfo> g_programs;

    struct VertexAttribState {
        bool enabled = false;
        GLint size = 0;
        GLenum type = 0;
        bool normalized = false;
        GLsizei stride = 0;
        const void* pointer = nullptr;
        size_t bufferOffset = 0;
        bool usesBuffer = false;
    };
    VertexAttribState g_attribs[8];

    // VAO support: map VAO id -> attribute states
    std::unordered_map<GLuint, std::array<VertexAttribState,8>> g_vertexArrays;
    GLuint g_currentVAO = 0;

    // buffer-range bindings (index -> (buffer, offset, size))
    struct BufferRange { GLuint buffer=0; size_t offset=0; size_t size=0; };
    std::unordered_map<GLuint, BufferRange> g_bufferRanges;

    std::vector<uint32_t> g_backbuffer;
    int g_width = 0, g_height = 0;
    std::mutex g_mutex;

    GLfloat g_clearColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};
}

extern "C" {

void wiiu_gl_initialize(int width, int height)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    g_width = width; g_height = height;
    g_backbuffer.assign(static_cast<size_t>(width) * static_cast<size_t>(height), 0);
    // Initialize GX2 renderer if available; fall back silently to software path
    if (rmx::WiiUGfx::isGX2Active())
    {
        bool ok = rmx::GX2Renderer::instance().initialize(width, height);
        (void)ok;
    }
}

void wiiu_gl_shutdown()
{
    std::lock_guard<std::mutex> lock(g_mutex);
    // Destroy any WHB textures we created
    for (auto &kv : g_textures)
    {
        if (kv.second.whbHandle != 0)
        {
#if GL_COMPAT_HAS_WHB
            WHBGfxDestroyTexture(kv.second.whbHandle);
#endif
            kv.second.whbHandle = 0;
        }
    }
    g_textures.clear();
    g_buffers.clear();
    g_backbuffer.clear();
    // Shutdown GX2 renderer if active
    if (rmx::WiiUGfx::isGX2Active()) rmx::GX2Renderer::instance().shutdown();
}

void wiiu_gl_present()
{
    std::lock_guard<std::mutex> lock(g_mutex);
    if (g_backbuffer.empty()) return;
    rmx::WiiUGfx::present(g_backbuffer.data(), g_width, g_height);
}

// Textures
void glGenTextures(GLsizei n, GLuint* textures)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    for (GLsizei i = 0; i < n; ++i)
        textures[i] = g_nextTexture++;
}

void glDeleteTextures(GLsizei n, const GLuint* textures)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    for (GLsizei i = 0; i < n; ++i)
    {
        auto it = g_textures.find(textures[i]);
        if (it != g_textures.end())
        {
            if (it->second.whbHandle != 0)
            {
#if GL_COMPAT_HAS_WHB
                WHBGfxDestroyTexture(it->second.whbHandle);
#endif
                it->second.whbHandle = 0;
            }
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
    // Bind WHB texture if present
    if (rmx::WiiUGfx::isGX2Active() && texture != 0)
    {
        auto it = g_textures.find(texture);
        if (it != g_textures.end() && it->second.whbHandle != 0)
        {
#if GL_COMPAT_HAS_WHB
            WHBGfxBindTexture(it->second.whbHandle);
#endif
        }
        else
        {
#if GL_COMPAT_HAS_WHB
            WHBGfxUnbindTexture();
#endif
        }
    }
}

void glActiveTexture(GLenum texture)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    const int unit = static_cast<int>(texture - GL_TEXTURE0);
    if (unit >= 0 && unit < 16) g_activeTextureUnit = unit;
}

static void uploadRGBAtoTex(TextureData& t, const void* pixels, int width, int height)
{
    t.w = width; t.h = height;
    t.pixels.resize(static_cast<size_t>(width) * static_cast<size_t>(height));
    if (!pixels) return;
    const uint8_t* src = static_cast<const uint8_t*>(pixels);
    size_t idx = 0;
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            uint8_t r = src[idx*4 + 0];
            uint8_t g = src[idx*4 + 1];
            uint8_t b = src[idx*4 + 2];
            uint8_t a = src[idx*4 + 3];
            t.pixels[y * width + x] = (static_cast<uint32_t>(a) << 24) |
                                      (static_cast<uint32_t>(b) << 16) |
                                      (static_cast<uint32_t>(g) << 8)  |
                                      (static_cast<uint32_t>(r));
            ++idx;
        }
    }
}

void glTexImage2D(GLenum, GLint, GLint, GLsizei width, GLsizei height, GLint, GLenum format, GLenum type, const void* pixels)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    if (g_boundTexture == 0) { RMX_LOG_WARNING("glTexImage2D: no bound texture"); return; }

    // Basic bounds/sanity checks to avoid malformed uploads
    if (width <= 0 || height <= 0)
    {
        RMX_LOG_ERROR("glTexImage2D: invalid dimensions " << width << "x" << height);
        return;
    }
    const int MAX_DIM = 8192;
    if (width > MAX_DIM || height > MAX_DIM)
    {
        RMX_LOG_ERROR("glTexImage2D: dimensions too large " << width << "x" << height);
        return;
    }
    TextureData& t = g_textures[g_boundTexture];
    // Only implement common case: GL_RGBA + GL_UNSIGNED_BYTE
    if (format == GL_RGBA && type == GL_UNSIGNED_BYTE)
    {
        // Prevent huge allocations
        const size_t pixelsCount = static_cast<size_t>(width) * static_cast<size_t>(height);
        if (pixelsCount == 0 || pixelsCount > (1u<<28))
        {
            RMX_LOG_ERROR("glTexImage2D: pixel count invalid or too large: " << pixelsCount);
            return;
        }
        uploadRGBAtoTex(t, pixels, width, height);
        // Attempt WHB/GX2 upload if available
        if (rmx::WiiUGfx::isGX2Active())
        {
#if GL_COMPAT_HAS_WHB
            if (t.whbHandle != 0) { WHBGfxDestroyTexture(t.whbHandle); t.whbHandle = 0; }
            if (width > 0 && height > 0 && pixels)
            {
                int h = WHBGfxCreateTexture(width, height, WHB_GX2_FORMAT_RGBA8, pixels);
                if (h >= 0) t.whbHandle = h;
            }
#endif
        }
    }
    else if (format == GL_LUMINANCE && type == GL_UNSIGNED_BYTE)
    {
        t.w = width; t.h = height;
        t.pixels.resize(static_cast<size_t>(width) * static_cast<size_t>(height));
        const uint8_t* src = static_cast<const uint8_t*>(pixels);
        for (int i = 0; i < width * height; ++i)
        {
            uint8_t v = src[i];
            t.pixels[i] = (0xFFu<<24) | (static_cast<uint32_t>(v)<<16) | (static_cast<uint32_t>(v)<<8) | (static_cast<uint32_t>(v));
        }
    }
    else
    {
        // Fallback: allocate, but don't initialize
        t.w = width; t.h = height; t.pixels.resize(static_cast<size_t>(width) * static_cast<size_t>(height));
    }
    // If we have an associated WHB texture, update it too
    if (rmx::WiiUGfx::isGX2Active() && t.whbHandle != 0)
    {
#if GL_COMPAT_HAS_WHB
        // WHB helper supports updating a subrect; update entire texture at 0,0
        WHBGfxUpdateTexture(t.whbHandle, 0, 0, width, height, pixels);
#endif
    }
}

void glTexSubImage2D(GLenum, GLint, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void* pixels)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    if (g_boundTexture == 0) return;
    auto it = g_textures.find(g_boundTexture);
    if (it == g_textures.end()) return;
    TextureData& t = it->second;
    if (t.pixels.empty()) { RMX_LOG_WARNING("glTexSubImage2D: texture has no pixels"); return; }
    if (xoffset < 0 || yoffset < 0) { RMX_LOG_ERROR("glTexSubImage2D: negative offset ("<<xoffset<<","<<yoffset<<")"); return; }
    if (xoffset + width > t.w || yoffset + height > t.h) { RMX_LOG_ERROR("glTexSubImage2D: subimage out of bounds: tex="<<g_boundTexture<<" rect=("<<xoffset<<","<<yoffset<<","<<width<<","<<height<<") texSize=("<<t.w<<","<<t.h<<")"); return; }
    if (format == GL_RGBA && type == GL_UNSIGNED_BYTE)
    {
        const uint8_t* src = static_cast<const uint8_t*>(pixels);
        for (int row = 0; row < height; ++row)
        {
            int dstY = yoffset + row;
            if (dstY < 0 || dstY >= t.h) continue;
            for (int col = 0; col < width; ++col)
            {
                int dstX = xoffset + col;
                if (dstX < 0 || dstX >= t.w) continue;
                size_t sidx = static_cast<size_t>(row * width + col) * 4;
                uint8_t r = src[sidx+0];
                uint8_t g = src[sidx+1];
                uint8_t b = src[sidx+2];
                uint8_t a = src[sidx+3];
                t.pixels[dstY * t.w + dstX] = (static_cast<uint32_t>(a) << 24) |
                                              (static_cast<uint32_t>(b) << 16) |
                                              (static_cast<uint32_t>(g) << 8)  |
                                              (static_cast<uint32_t>(r));
            }
        }
    }
    else if (format == GL_LUMINANCE && type == GL_UNSIGNED_BYTE)
    {
        const uint8_t* src = static_cast<const uint8_t*>(pixels);
        for (int row = 0; row < height; ++row)
        {
            int dstY = yoffset + row;
            if (dstY < 0 || dstY >= t.h) continue;
            for (int col = 0; col < width; ++col)
            {
                int dstX = xoffset + col;
                if (dstX < 0 || dstX >= t.w) continue;
                size_t sidx = static_cast<size_t>(row * width + col);
                uint8_t v = src[sidx];
                t.pixels[dstY * t.w + dstX] = (0xFFu<<24) | (static_cast<uint32_t>(v)<<16) | (static_cast<uint32_t>(v)<<8) | (static_cast<uint32_t>(v));
            }
        }
    }
}

void glTexParameteri(GLenum /*target*/, GLenum pname, GLint param)
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

GLboolean glIsTexture(GLuint texture)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    return g_textures.find(texture) != g_textures.end() ? GL_TRUE : GL_FALSE;
}

// Buffers
void glGenBuffers(GLsizei n, GLuint* buffers)
{
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
    if (target == GL_ARRAY_BUFFER) g_boundArrayBuffer = buffer;
    else if (target == GL_ELEMENT_ARRAY_BUFFER) g_boundElementArrayBuffer = buffer;
    else if (target == GL_TEXTURE_BUFFER) g_boundTextureBuffer = buffer;
}

void glBufferData(GLenum target, GLsizeiptr size, const void* data, GLenum)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    if (target == GL_ARRAY_BUFFER && g_boundArrayBuffer != 0)
    {
        auto& buf = g_buffers[g_boundArrayBuffer];
        buf.resize(static_cast<size_t>(size));
        if (data && size > 0) std::memcpy(buf.data(), data, static_cast<size_t>(size));
    }
    else if (target == GL_ELEMENT_ARRAY_BUFFER && g_boundElementArrayBuffer != 0)
    {
        auto& buf = g_buffers[g_boundElementArrayBuffer];
        buf.resize(static_cast<size_t>(size));
        if (data && size > 0) std::memcpy(buf.data(), data, static_cast<size_t>(size));
    }
}

void glTexBuffer(GLenum target, GLenum internalFormat, GLuint buffer)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    if (target != GL_TEXTURE_BUFFER) return;
    // The currently bound texture is stored in g_boundTexture (set by glBindTexture)
    if (g_boundTexture == 0) return;
    auto it = g_buffers.find(buffer);
    if (it == g_buffers.end()) return;
    auto pit = g_textures.find(g_boundTexture);
    if (pit == g_textures.end()) return;
    TextureData& t = pit->second;
    // Pack buffer bytes into luminance pixels
    const std::vector<uint8_t>& buf = it->second;
    size_t width = buf.size();
    if (width == 0) width = 1;
    t.w = static_cast<int>(width);
    t.h = 1;
    t.pixels.resize(width);
    for (size_t i = 0; i < width; ++i)
    {
        uint8_t v = buf[i];
        t.pixels[i] = (0xFFu<<24) | (static_cast<uint32_t>(v)<<16) | (static_cast<uint32_t>(v)<<8) | (static_cast<uint32_t>(v));
    }
}

void glTexBufferRange(GLenum target, GLint internalFormat, GLuint buffer, GLintptr offset, GLsizeiptr size)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    if (target != GL_TEXTURE_BUFFER) return;
    if (g_boundTexture == 0) return;
    auto it = g_buffers.find(buffer);
    if (it == g_buffers.end()) return;
    auto pit = g_textures.find(g_boundTexture);
    if (pit == g_textures.end()) return;
    TextureData& t = pit->second;
    const std::vector<uint8_t>& buf = it->second;
    size_t off = static_cast<size_t>(offset);
    size_t sz = static_cast<size_t>(size);
    if (off > buf.size()) off = buf.size();
    if (off + sz > buf.size()) sz = buf.size() - off;
    size_t width = sz == 0 ? 1 : sz;
    t.w = static_cast<int>(width);
    t.h = 1;
    t.pixels.resize(width);
    for (size_t i = 0; i < width; ++i)
    {
        uint8_t v = buf[off + i];
        t.pixels[i] = (0xFFu<<24) | (static_cast<uint32_t>(v)<<16) | (static_cast<uint32_t>(v)<<8) | (static_cast<uint32_t>(v));
    }
}


// Vertex arrays / attributes
void glGenVertexArrays(GLsizei n, GLuint* arrays)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    for (GLsizei i = 0; i < n; ++i)
    {
        GLuint id = g_nextBuffer++;
        arrays[i] = id;
        g_vertexArrays[id] = std::array<VertexAttribState,8>();
    }
}
void glBindVertexArray(GLuint array)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    g_currentVAO = array;
    if (array == 0) return;
    if (g_vertexArrays.find(array) == g_vertexArrays.end())
        g_vertexArrays[array] = std::array<VertexAttribState,8>();
    for (size_t i = 0; i < 8; ++i) g_attribs[i] = g_vertexArrays[array][i];
}
void glDeleteVertexArrays(GLsizei n, const GLuint* arrays)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    for (GLsizei i = 0; i < n; ++i) g_vertexArrays.erase(arrays[i]);
    if (g_currentVAO != 0 && g_vertexArrays.find(g_currentVAO) == g_vertexArrays.end()) g_currentVAO = 0;
}

void glVertexAttribPointer(GLuint index, GLint size, GLenum type, bool normalized, GLsizei stride, const void* pointer)
{
    if (index >= 8) return;
    VertexAttribState* a = nullptr;
    if (g_currentVAO != 0)
    {
        a = &g_vertexArrays[g_currentVAO][index];
    }
    else
    {
        a = &g_attribs[index];
    }
    a->size = size; a->type = type; a->normalized = normalized; a->stride = stride;
    if (g_boundArrayBuffer != 0)
    {
        a->usesBuffer = true;
        a->bufferOffset = reinterpret_cast<size_t>(pointer);
        a->pointer = nullptr;
    }
    else
    {
        a->usesBuffer = false;
        a->pointer = pointer;
        a->bufferOffset = 0;
    }
}

void glEnableVertexAttribArray(GLuint index) { if (index < 8) { if (g_currentVAO != 0) g_vertexArrays[g_currentVAO][index].enabled = true; else g_attribs[index].enabled = true; } }
void glDisableVertexAttribArray(GLuint index) { if (index < 8) { if (g_currentVAO != 0) g_vertexArrays[g_currentVAO][index].enabled = false; else g_attribs[index].enabled = false; } }

// Draw calls: lightweight software fallback only (triangles with pos/uv)
static inline std::pair<int,int> ndcToScreen(float nx, float ny)
{
    float sx = (nx * 0.5f + 0.5f) * static_cast<float>(g_width);
    float sy = (1.0f - (ny * 0.5f + 0.5f)) * static_cast<float>(g_height);
    return std::make_pair(static_cast<int>(std::floor(sx)), static_cast<int>(std::floor(sy)));
}

void glDrawArrays(GLenum, GLint, GLsizei)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    if (g_backbuffer.empty()) return;

    // Simple fixed-vertex count fallback: attempt to interpret first two attribs as pos(x,y) and uv(u,v)
    // We can't infer count reliably here; the engine typically submits full quads, so attempt 6 verts if available in bound buffer.
    const int assumedCount = 6;
    std::vector<float> posbuf; posbuf.reserve(assumedCount * 4);

    // Build vertices from attribute 0 (pos) and 1 (uv)
    struct V { float x,y,u,v; };
    std::vector<V> verts; verts.reserve(assumedCount);

    for (int i = 0; i < assumedCount; ++i)
    {
        V v{};
        if (g_attribs[0].enabled)
        {
            const VertexAttribState& a = g_attribs[0];
            const float* src = nullptr;
            if (a.usesBuffer && g_boundArrayBuffer != 0)
            {
                auto it = g_buffers.find(g_boundArrayBuffer);
                if (it != g_buffers.end())
                {
                    size_t base = a.bufferOffset + static_cast<size_t>(i) * (a.stride ? a.stride : (a.size * sizeof(float)));
                    if (base + a.size * sizeof(float) <= it->second.size())
                        src = reinterpret_cast<const float*>(it->second.data() + base);
                }
            }
            else if (a.pointer)
            {
                const char* base = reinterpret_cast<const char*>(a.pointer);
                size_t offset = static_cast<size_t>(i) * (a.stride ? a.stride : (a.size * sizeof(float)));
                src = reinterpret_cast<const float*>(base + offset);
            }
            if (src) { v.x = src[0]; v.y = src[1]; }
        }
        if (g_attribs[1].enabled)
        {
            const VertexAttribState& a = g_attribs[1];
            const float* src = nullptr;
            if (a.usesBuffer && g_boundArrayBuffer != 0)
            {
                auto it = g_buffers.find(g_boundArrayBuffer);
                if (it != g_buffers.end())
                {
                    size_t base = a.bufferOffset + static_cast<size_t>(i) * (a.stride ? a.stride : (a.size * sizeof(float)));
                    if (base + a.size * sizeof(float) <= it->second.size())
                        src = reinterpret_cast<const float*>(it->second.data() + base);
                }
            }
            else if (a.pointer)
            {
                const char* base = reinterpret_cast<const char*>(a.pointer);
                size_t offset = static_cast<size_t>(i) * (a.stride ? a.stride : (a.size * sizeof(float)));
                src = reinterpret_cast<const float*>(base + offset);
            }
            if (src) { v.u = src[0]; v.v = src[1]; }
        }
        verts.push_back(v);
    }

    // Fast-path: if GX2 is active and texture has a WHB handle, use WHB textured triangle draws
    if (rmx::WiiUGfx::isGX2Active() && g_boundTexture != 0)
    {
        auto itTex = g_textures.find(g_boundTexture);
        if (itTex != g_textures.end() && itTex->second.whbHandle != 0)
        {
            // Draw each triangle via WHB helper
            for (size_t tri = 0; tri + 2 < verts.size(); tri += 3)
            {
                const V& v0 = verts[tri+0];
                const V& v1 = verts[tri+1];
                const V& v2 = verts[tri+2];
                auto p0 = ndcToScreen(v0.x, v0.y);
                auto p1 = ndcToScreen(v1.x, v1.y);
                auto p2 = ndcToScreen(v2.x, v2.y);
#if GL_COMPAT_HAS_WHB
                WHBGfxDrawTexturedTriangle(p0.first, p0.second, v0.u, v0.v,
                                           p1.first, p1.second, v1.u, v1.v,
                                           p2.first, p2.second, v2.u, v2.v);
#endif
            }
            return; // skip software rasterizer
        }
    }

    auto sampleTexture = [&](float u, float v)->uint32_t {
        if (g_boundTexture == 0) return 0;
        auto it = g_textures.find(g_boundTexture);
        if (it == g_textures.end()) return 0;
        const TextureData& t = it->second;
        if (t.pixels.empty()) return 0;
        int tx = static_cast<int>(u * (t.w - 1) + 0.5f);
        int ty = static_cast<int>(v * (t.h - 1) + 0.5f);
        tx = std::clamp(tx, 0, t.w - 1);
        ty = std::clamp(ty, 0, t.h - 1);
        return t.pixels[ty * t.w + tx];
    };

    // Rasterize triangles (very basic)
    for (size_t tri = 0; tri + 2 < verts.size(); tri += 3)
    {
        const V& v0 = verts[tri+0];
        const V& v1 = verts[tri+1];
        const V& v2 = verts[tri+2];
        auto p0 = ndcToScreen(v0.x, v0.y);
        auto p1 = ndcToScreen(v1.x, v1.y);
        auto p2 = ndcToScreen(v2.x, v2.y);

        int minx = std::max(0, std::min(std::min(p0.first, p1.first), p2.first));
        int maxx = std::min(g_width - 1, std::max(std::max(p0.first, p1.first), p2.first));
        int miny = std::max(0, std::min(std::min(p0.second, p1.second), p2.second));
        int maxy = std::min(g_height - 1, std::max(std::max(p0.second, p1.second), p2.second));

        const float x0 = static_cast<float>(p0.first);
        const float y0 = static_cast<float>(p0.second);
        const float x1 = static_cast<float>(p1.first);
        const float y1 = static_cast<float>(p1.second);
        const float x2 = static_cast<float>(p2.first);
        const float y2 = static_cast<float>(p2.second);

        const float denom = (y1 - y2) * (x0 - x2) + (x2 - x1) * (y0 - y2);
        if (std::fabs(denom) < 1e-6f) continue;

        for (int y = miny; y <= maxy; ++y)
        {
            for (int x = minx; x <= maxx; ++x)
            {
                const float px = static_cast<float>(x);
                const float py = static_cast<float>(y);
                const float w0 = ((y1 - y2) * (px - x2) + (x2 - x1) * (py - y2)) / denom;
                const float w1 = ((y2 - y0) * (px - x2) + (x0 - x2) * (py - y2)) / denom;
                const float w2 = 1.0f - w0 - w1;
                if (w0 >= 0 && w1 >= 0 && w2 >= 0)
                {
                    const float tu = w0 * v0.u + w1 * v1.u + w2 * v2.u;
                    const float tv = w0 * v0.v + w1 * v1.v + w2 * v2.v;
                    uint32_t color = sampleTexture(tu, tv);
                    g_backbuffer[y * g_width + x] = color;
                }
            }
        }
    }
}

// Indexed draws
void glDrawElements(GLenum, GLsizei count, GLenum type, const void* indices)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    if (g_backbuffer.empty() || count <= 0) return;

    std::vector<uint32_t> idxs; idxs.reserve(static_cast<size_t>(count));
    if (g_boundElementArrayBuffer != 0)
    {
        auto it = g_buffers.find(g_boundElementArrayBuffer);
        if (it == g_buffers.end()) return;
        const std::vector<uint8_t>& buf = it->second;
        if (type == GL_UNSIGNED_SHORT)
        {
            const uint16_t* data = reinterpret_cast<const uint16_t*>(buf.data());
            size_t n = buf.size() / sizeof(uint16_t);
            for (size_t i = 0; i < static_cast<size_t>(count) && i < n; ++i) idxs.push_back(data[i]);
        }
        else if (type == GL_UNSIGNED_INT)
        {
            const uint32_t* data = reinterpret_cast<const uint32_t*>(buf.data());
            size_t n = buf.size() / sizeof(uint32_t);
            for (size_t i = 0; i < static_cast<size_t>(count) && i < n; ++i) idxs.push_back(data[i]);
        }
        else
        {
            const uint8_t* data = reinterpret_cast<const uint8_t*>(buf.data());
            size_t n = buf.size();
            for (size_t i = 0; i < static_cast<size_t>(count) && i < n; ++i) idxs.push_back(data[i]);
        }
    }
    else
    {
        if (!indices) return;
        if (type == GL_UNSIGNED_SHORT)
        {
            const uint16_t* data = reinterpret_cast<const uint16_t*>(indices);
            for (int i = 0; i < count; ++i) idxs.push_back(data[i]);
        }
        else if (type == GL_UNSIGNED_INT)
        {
            const uint32_t* data = reinterpret_cast<const uint32_t*>(indices);
            for (int i = 0; i < count; ++i) idxs.push_back(data[i]);
        }
        else
        {
            const uint8_t* data = reinterpret_cast<const uint8_t*>(indices);
            for (int i = 0; i < count; ++i) idxs.push_back(data[i]);
        }
    }

    // Build vertex list then reuse glDrawArrays rasterization logic by creating small temp arrays
    struct V { float x,y,u,v; };
    std::vector<V> verts; verts.reserve(idxs.size());

    for (size_t i = 0; i < idxs.size(); ++i)
    {
        uint32_t idx = idxs[i];
        V v{};
        if (g_attribs[0].enabled)
        {
            const VertexAttribState& a = g_attribs[0];
            const float* src = nullptr;
            if (a.usesBuffer && g_boundArrayBuffer != 0)
            {
                auto it = g_buffers.find(g_boundArrayBuffer);
                if (it != g_buffers.end())
                {
                    size_t base = a.bufferOffset + static_cast<size_t>(idx) * (a.stride ? a.stride : (a.size * sizeof(float)));
                    if (base + a.size * sizeof(float) <= it->second.size()) src = reinterpret_cast<const float*>(it->second.data() + base);
                }
            }
            else if (a.pointer)
            {
                const char* base = reinterpret_cast<const char*>(a.pointer);
                size_t offset = static_cast<size_t>(idx) * (a.stride ? a.stride : (a.size * sizeof(float)));
                src = reinterpret_cast<const float*>(base + offset);
            }
            if (src) { v.x = src[0]; v.y = src[1]; }
        }
        if (g_attribs[1].enabled)
        {
            const VertexAttribState& a = g_attribs[1];
            const float* src = nullptr;
            if (a.usesBuffer && g_boundArrayBuffer != 0)
            {
                auto it = g_buffers.find(g_boundArrayBuffer);
                if (it != g_buffers.end())
                {
                    size_t base = a.bufferOffset + static_cast<size_t>(idx) * (a.stride ? a.stride : (a.size * sizeof(float)));
                    if (base + a.size * sizeof(float) <= it->second.size()) src = reinterpret_cast<const float*>(it->second.data() + base);
                }
            }
            else if (a.pointer)
            {
                const char* base = reinterpret_cast<const char*>(a.pointer);
                size_t offset = static_cast<size_t>(idx) * (a.stride ? a.stride : (a.size * sizeof(float)));
                src = reinterpret_cast<const float*>(base + offset);
            }
            if (src) { v.u = src[0]; v.v = src[1]; }
        }
        verts.push_back(v);
    }

    // Rasterize verts in groups of 3
    // Fast-path: if GX2 is active and WHB texture exists, use WHB triangle draws
    if (rmx::WiiUGfx::isGX2Active() && g_boundTexture != 0)
    {
        auto itTex = g_textures.find(g_boundTexture);
        if (itTex != g_textures.end() && itTex->second.whbHandle != 0)
        {
            for (size_t tri = 0; tri + 2 < verts.size(); tri += 3)
            {
                const V& v0 = verts[tri+0];
                const V& v1 = verts[tri+1];
                const V& v2 = verts[tri+2];
                auto p0 = ndcToScreen(v0.x, v0.y);
                auto p1 = ndcToScreen(v1.x, v1.y);
                auto p2 = ndcToScreen(v2.x, v2.y);
#if GL_COMPAT_HAS_WHB
                WHBGfxDrawTexturedTriangle(p0.first, p0.second, v0.u, v0.v,
                                           p1.first, p1.second, v1.u, v1.v,
                                           p2.first, p2.second, v2.u, v2.v);
#endif
            }
            return;
        }
    }
    for (size_t tri = 0; tri + 2 < verts.size(); tri += 3)
    {
        const V& v0 = verts[tri+0];
        const V& v1 = verts[tri+1];
        const V& v2 = verts[tri+2];
        auto p0 = ndcToScreen(v0.x, v0.y);
        auto p1 = ndcToScreen(v1.x, v1.y);
        auto p2 = ndcToScreen(v2.x, v2.y);

        int minx = std::max(0, std::min(std::min(p0.first, p1.first), p2.first));
        int maxx = std::min(g_width - 1, std::max(std::max(p0.first, p1.first), p2.first));
        int miny = std::max(0, std::min(std::min(p0.second, p1.second), p2.second));
        int maxy = std::min(g_height - 1, std::max(std::max(p0.second, p1.second), p2.second));

        const float x0 = static_cast<float>(p0.first);
        const float y0 = static_cast<float>(p0.second);
        const float x1 = static_cast<float>(p1.first);
        const float y1 = static_cast<float>(p1.second);
        const float x2 = static_cast<float>(p2.first);
        const float y2 = static_cast<float>(p2.second);

        const float denom = (y1 - y2) * (x0 - x2) + (x2 - x1) * (y0 - y2);
        if (std::fabs(denom) < 1e-6f) continue;

        for (int y = miny; y <= maxy; ++y)
        {
            for (int x = minx; x <= maxx; ++x)
            {
                const float px = static_cast<float>(x);
                const float py = static_cast<float>(y);
                const float w0 = ((y1 - y2) * (px - x2) + (x2 - x1) * (py - y2)) / denom;
                const float w1 = ((y2 - y0) * (px - x2) + (x0 - x2) * (py - y2)) / denom;
                const float w2 = 1.0f - w0 - w1;
                if (w0 >= 0 && w1 >= 0 && w2 >= 0)
                {
                    const float tu = w0 * v0.u + w1 * v1.u + w2 * v2.u;
                    const float tv = w0 * v0.v + w1 * v1.v + w2 * v2.v;
                    uint32_t color = 0;
                    if (g_boundTexture != 0)
                    {
                        auto it = g_textures.find(g_boundTexture);
                        if (it != g_textures.end())
                        {
                            const TextureData& t = it->second;
                            if (!t.pixels.empty())
                            {
                                int tx = static_cast<int>(tu * (t.w - 1) + 0.5f);
                                int ty = static_cast<int>(tv * (t.h - 1) + 0.5f);
                                tx = std::clamp(tx, 0, t.w - 1);
                                ty = std::clamp(ty, 0, t.h - 1);
                                color = t.pixels[ty * t.w + tx];
                            }
                        }
                    }
                    g_backbuffer[y * g_width + x] = color;
                }
            }
        }
    }
}

void glDrawElementsInstanced(GLenum mode, GLsizei count, GLenum type, const void* indices, GLsizei instancecount)
{
    (void)mode; (void)instancecount; glDrawElements(mode, count, type, indices);
}

void glDrawRangeElements(GLenum mode, GLuint, GLuint, GLsizei count, GLenum type, const void* indices)
{
    (void)mode; glDrawElements(mode, count, type, indices);
}

// Remaining stubs
void glEnable(GLenum) {}
void glDisable(GLenum) {}
void glBlendFunc(GLenum, GLenum) {}
void glViewport(GLint, GLint, GLsizei, GLsizei) {}
void glClearColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a) { g_clearColor[0]=r; g_clearColor[1]=g; g_clearColor[2]=b; g_clearColor[3]=a; }
void glClear(GLenum) { std::fill(g_backbuffer.begin(), g_backbuffer.end(), (static_cast<uint32_t>(g_clearColor[3]*255.0f)<<24) | (static_cast<uint32_t>(g_clearColor[2]*255.0f)<<16) | (static_cast<uint32_t>(g_clearColor[1]*255.0f)<<8) | (static_cast<uint32_t>(g_clearColor[0]*255.0f))); }

// Minimal shader/program/uniform stubs
GLuint glCreateShader(GLenum)
{
    static GLuint s = 1;
    g_shaderSources[s] = std::string();
    return s++;
}
void glShaderSource(GLuint shader, GLsizei count, const char** string, const GLint* length)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    std::string &out = g_shaderSources[shader];
    out.clear();
    for (GLsizei i = 0; i < count; ++i)
    {
        if (length && length[i] > 0) out.append(string[i], length[i]);
        else out.append(string[i]);
    }
}
void glCompileShader(GLuint) { /* no-op: accept source as-is */ }
GLuint glCreateProgram()
{
    static GLuint p = 1;
    g_programs[p] = ProgramInfo();
    return p++;
}
void glAttachShader(GLuint program, GLuint shader)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    auto it = g_programs.find(program);
    if (it == g_programs.end()) return;
    it->second.attachedShaders.push_back(shader);
}
// Very small/link stage: parse attached shader sources for 'uniform <type> <name>;' declarations and register locations
void glLinkProgram(GLuint program)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    auto pit = g_programs.find(program);
    if (pit == g_programs.end()) return;
    ProgramInfo &info = pit->second;
    for (GLuint s : info.attachedShaders)
    {
        auto sit = g_shaderSources.find(s);
        if (sit == g_shaderSources.end()) continue;
        const std::string &src = sit->second;
        // naive parse: look for "uniform" tokens
        size_t pos = 0;
        while (true)
        {
            pos = src.find("uniform", pos);
            if (pos == std::string::npos) break;
            // find end of line
            size_t lineEnd = src.find(';', pos);
            if (lineEnd == std::string::npos) break;
            std::string decl = src.substr(pos, lineEnd - pos);
            // split decl into words
            std::vector<std::string> words;
            size_t p = 0;
            while (p < decl.size())
            {
                while (p < decl.size() && std::isspace((unsigned char)decl[p])) ++p;
                size_t q = p;
                while (q < decl.size() && !std::isspace((unsigned char)decl[q])) ++q;
                if (q > p) words.push_back(decl.substr(p, q-p));
                p = q;
            }
            if (words.size() >= 3)
            {
                std::string name = words[2];
                // strip possible array suffixes
                size_t br = name.find('[');
                if (br != std::string::npos) name = name.substr(0, br);
                if (info.uniformLocations.find(name) == info.uniformLocations.end())
                {
                    info.uniformLocations[name] = info.nextLocation++;
                }
            }
            pos = lineEnd + 1;
        }
    }
}
void glUseProgram(GLuint program)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    g_currentProgram = program;
}
GLint glGetUniformLocation(GLuint program, const char* name)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    auto it = g_programs.find(program);
    if (it == g_programs.end() || !name) return -1;
    auto uit = it->second.uniformLocations.find(std::string(name));
    if (uit == it->second.uniformLocations.end()) return -1;
    return uit->second;
}
GLint glGetAttribLocation(GLuint, const char*) { return -1; }
void glUniform1i(GLint location, GLint v0)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    if (g_currentProgram == 0) return;
    auto &info = g_programs[g_currentProgram];
    info.uniformInts[location] = std::vector<int>{v0};
}
void glUniform2iv(GLint location, GLsizei count, const GLint* value)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    if (g_currentProgram == 0) return;
    g_programs[g_currentProgram].uniformInts[location] = std::vector<int>(value, value + count*2);
}
void glUniform3iv(GLint location, GLsizei count, const GLint* value)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    if (g_currentProgram == 0) return;
    g_programs[g_currentProgram].uniformInts[location] = std::vector<int>(value, value + count*3);
}
void glUniform4iv(GLint location, GLsizei count, const GLint* value)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    if (g_currentProgram == 0) return;
    g_programs[g_currentProgram].uniformInts[location] = std::vector<int>(value, value + count*4);
}
void glUniform1f(GLint location, GLfloat v0)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    if (g_currentProgram == 0) return;
    g_programs[g_currentProgram].uniformFloats[location] = std::vector<float>{v0};
}
void glUniform2fv(GLint location, GLsizei count, const GLfloat* value)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    if (g_currentProgram == 0) return;
    g_programs[g_currentProgram].uniformFloats[location] = std::vector<float>(value, value + count*2);
}
void glUniform3fv(GLint location, GLsizei count, const GLfloat* value)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    if (g_currentProgram == 0) return;
    g_programs[g_currentProgram].uniformFloats[location] = std::vector<float>(value, value + count*3);
}
void glUniform4fv(GLint location, GLsizei count, const GLfloat* value)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    if (g_currentProgram == 0) return;
    g_programs[g_currentProgram].uniformFloats[location] = std::vector<float>(value, value + count*4);
}

// helper to get or create ProgramInfo ref by program id (used above)
static ProgramInfo& info_for_location(GLuint program, GLint location)
{
    return g_programs[program];
}
GLenum glGetError() { return 0; }
void glDebugMessageCallback(GLDEBUGPROC, const void*) {}
GLboolean glUnmapBuffer(GLenum target)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    (void)target;
    return GL_TRUE;
}

void* glMapBuffer(GLenum target, GLenum access)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    (void)access;
    GLuint bufId = 0;
    if (target == GL_ARRAY_BUFFER) bufId = g_boundArrayBuffer;
    else if (target == GL_ELEMENT_ARRAY_BUFFER) bufId = g_boundElementArrayBuffer;
    else if (target == GL_TEXTURE_BUFFER) bufId = g_boundTextureBuffer;
    if (bufId == 0) return nullptr;
    auto it = g_buffers.find(bufId);
    if (it == g_buffers.end()) return nullptr;
    // Return pointer to buffer memory (caller must not rely on long-term mapping semantics)
    return reinterpret_cast<void*>(it->second.data());
}

void glBindBufferRange(GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    (void)target;
    BufferRange br; br.buffer = buffer; br.offset = static_cast<size_t>(offset); br.size = static_cast<size_t>(size);
    g_bufferRanges[index] = br;
}

} // extern "C"
