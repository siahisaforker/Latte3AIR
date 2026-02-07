#pragma once
// Wii U VBO/Shader management utilities.
// Provides helper functions to manage vertex buffer objects and shader
// pipelines on the Wii U, bridging the engine's OpenGL-style calls
// (routed through gl_compat) to GX2.

#include <cstdint>
#include <vector>

namespace wiiu_gpu {

/// VBO handle type (maps to gl_compat buffer IDs).
using VBOHandle = uint32_t;

/// Vertex format descriptors matching engine's common vertex layouts.
enum class VertexFormat {
    POS2_UV2,       // 2D position + 2D texcoord (16 bytes/vert)
    POS2_UV2_COL4,  // 2D position + 2D texcoord + RGBA colour (20 bytes/vert)
    POS3_UV2,       // 3D position + 2D texcoord (20 bytes/vert)
    POS3_UV2_COL4,  // 3D position + 2D texcoord + RGBA colour (24 bytes/vert)
    POS2_COL4,      // 2D position + RGBA colour (12 bytes/vert)
};

/// Get the byte stride of a vertex format.
int vertexStride(VertexFormat fmt);

/// Simple vertex batch — accumulate draw data CPU-side, then flush to GX2.
class VertexBatch {
public:
    VertexBatch();
    ~VertexBatch();

    /// Reset the batch for a new frame.
    void begin(VertexFormat fmt);

    /// Push a vertex (data must be `vertexStride(fmt)` bytes).
    void pushVertex(const void* vertData);

    /// Push a triangle (3 vertices).
    void pushTriangle(const void* v0, const void* v1, const void* v2);

    /// Push a textured quad as 2 triangles (4 vertices, CCW winding).
    void pushQuad(const void* v0, const void* v1, const void* v2, const void* v3);

    /// Push raw vertex data.
    void pushRaw(const void* data, int numVerts);

    /// Flush accumulated vertices to the software rasterizer / GX2.
    /// texHandle = gl_compat texture ID (0 = untextured).
    void flush(int texHandle = 0);

    /// Get number of vertices accumulated so far.
    int vertexCount() const { return mVertexCount; }

private:
    std::vector<uint8_t> mData;
    VertexFormat mFormat = VertexFormat::POS2_UV2;
    int mStride = 16;
    int mVertexCount = 0;
};

/// Shader effect IDs (CPU-side replacements for the engine's GLSL shaders).
enum class ShaderEffect {
    NONE,
    COPY,            // Simple screen copy
    TEXTURED_RECT,   // Textured rectangle with UV
    COLORED_RECT,    // Solid-colour rectangle
    INDEXED_RECT,    // Palette-indexed draw (used by Sonic 3 rendering)
    PALETTE_SPRITE,  // Paletted sprite
    VDP_SPRITE,      // VDP sprite layer
    PLANE,           // Background plane
    POST_FX_BLUR,    // Gaussian blur post-FX
    DEBUG_PLANE,     // Debug overlay
};

/// Set the active shader effect (selects CPU-path code in gl_compat draw).
void setActiveEffect(ShaderEffect effect);

/// Get current active effect.
ShaderEffect getActiveEffect();

/// Upload a palette for indexed drawing (up to 256 RGBA entries).
void uploadPalette(const uint32_t* rgba, int count);

} // namespace wiiu_gpu
