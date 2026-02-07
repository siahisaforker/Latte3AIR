#include "wiiu_gpu.h"

#if defined(PLATFORM_WIIU)
#include "wiiu/gl_compat.h"
#include "wiiu/WiiUGfx.h"
#include "rmxmedia/framework/wiiu_shim_gx2.h"
#endif

#include <cstring>
#include <algorithm>

namespace {
    wiiu_gpu::ShaderEffect g_activeEffect = wiiu_gpu::ShaderEffect::NONE;
    uint32_t g_palette[256] = {};
    int g_paletteSize = 0;
}

namespace wiiu_gpu {

int vertexStride(VertexFormat fmt)
{
    switch (fmt) {
        case VertexFormat::POS2_UV2:       return 16;
        case VertexFormat::POS2_UV2_COL4:  return 20;
        case VertexFormat::POS3_UV2:       return 20;
        case VertexFormat::POS3_UV2_COL4:  return 24;
        case VertexFormat::POS2_COL4:      return 12;
    }
    return 16;
}

VertexBatch::VertexBatch() {}
VertexBatch::~VertexBatch() {}

void VertexBatch::begin(VertexFormat fmt)
{
    mFormat = fmt;
    mStride = vertexStride(fmt);
    mData.clear();
    mVertexCount = 0;
}

void VertexBatch::pushVertex(const void* vertData)
{
    size_t off = mData.size();
    mData.resize(off + mStride);
    std::memcpy(mData.data() + off, vertData, mStride);
    ++mVertexCount;
}

void VertexBatch::pushTriangle(const void* v0, const void* v1, const void* v2)
{
    pushVertex(v0);
    pushVertex(v1);
    pushVertex(v2);
}

void VertexBatch::pushQuad(const void* v0, const void* v1, const void* v2, const void* v3)
{
    // Two triangles: 0-1-2, 0-2-3
    pushTriangle(v0, v1, v2);
    pushTriangle(v0, v2, v3);
}

void VertexBatch::pushRaw(const void* data, int numVerts)
{
    size_t bytes = static_cast<size_t>(numVerts) * mStride;
    size_t off = mData.size();
    mData.resize(off + bytes);
    std::memcpy(mData.data() + off, data, bytes);
    mVertexCount += numVerts;
}

void VertexBatch::flush(int texHandle)
{
    if (mVertexCount < 3) { mData.clear(); mVertexCount = 0; return; }

#if defined(PLATFORM_WIIU)
    // Upload vertex data to gl_compat VBO and issue draw
    GLuint vbo = 0;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(mData.size()), mData.data(), GL_STREAM_DRAW);

    // Set vertex attribs based on format
    switch (mFormat) {
        case VertexFormat::POS2_UV2:
            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, mStride, reinterpret_cast<void*>(0));
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, mStride, reinterpret_cast<void*>(8));
            glEnableVertexAttribArray(1);
            break;
        case VertexFormat::POS2_UV2_COL4:
            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, mStride, reinterpret_cast<void*>(0));
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, mStride, reinterpret_cast<void*>(8));
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, mStride, reinterpret_cast<void*>(16));
            glEnableVertexAttribArray(2);
            break;
        case VertexFormat::POS2_COL4:
            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, mStride, reinterpret_cast<void*>(0));
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, mStride, reinterpret_cast<void*>(8));
            glEnableVertexAttribArray(2);
            break;
        default:
            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, mStride, reinterpret_cast<void*>(0));
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, mStride, reinterpret_cast<void*>(8));
            glEnableVertexAttribArray(1);
            break;
    }

    if (texHandle != 0)
        glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(texHandle));

    glDrawArrays(GL_TRIANGLES, 0, mVertexCount);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDeleteBuffers(1, &vbo);
#endif

    mData.clear();
    mVertexCount = 0;
}

void setActiveEffect(ShaderEffect effect)
{
    g_activeEffect = effect;
}

ShaderEffect getActiveEffect()
{
    return g_activeEffect;
}

void uploadPalette(const uint32_t* rgba, int count)
{
    if (!rgba || count <= 0) return;
    int n = std::min(count, 256);
    std::memcpy(g_palette, rgba, static_cast<size_t>(n) * sizeof(uint32_t));
    g_paletteSize = n;
}

} // namespace wiiu_gpu
