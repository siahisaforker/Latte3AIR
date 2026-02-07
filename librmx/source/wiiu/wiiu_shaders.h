/*
 * CPU-fallback shader pipeline for Wii U
 *
 * Since the Wii U's GX2 GPU cannot execute GLSL shaders directly,
 * this module provides CPU implementations of every engine shader.
 * At glLinkProgram time each program is identified by its uniform
 * signature, and at glDrawArrays time the appropriate C++ fragment
 * function is invoked per-pixel.
 */
#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <unordered_map>

namespace wiiu_shaders {

// ============================================================================
// Shader identification
// ============================================================================
enum class CpuShaderType
{
    UNKNOWN = 0,
    SIMPLE_COPY_SCREEN,
    SIMPLE_RECT_COLORED,
    SIMPLE_RECT_INDEXED,
    SIMPLE_RECT_TEXTURED,
    SIMPLE_RECT_TEXTURED_UV,
    SIMPLE_RECT_VERTEX_COLOR,
    SIMPLE_RECT_OVERDRAW,
    POST_FX_BLUR,
    RENDER_PLANE,
    RENDER_PLANE_HSCROLL,
    RENDER_PLANE_VSCROLL,
    RENDER_PLANE_NO_REPEAT,
    RENDER_VDP_SPRITE,
    RENDER_PALETTE_SPRITE,
    RENDER_COMPONENT_SPRITE,
    DEBUG_DRAW_PLANE,
};

const char* shaderTypeName(CpuShaderType t);

// ============================================================================
// Texture data supplied to shaders
// ============================================================================
struct ShaderTexture
{
    const uint32_t* pixels = nullptr;      // RGBA pixel data (ABGR packed)
    const uint8_t*  rawBytes = nullptr;    // raw buffer data (isamplerBuffer)
    int   width = 0;
    int   height = 0;
    int   bytesPerPixel = 1;               // 1 for R8UI, 2 for R16I/R16UI
    bool  isBufferTexture = false;

    // Read a single integer from a buffer texture at linear index
    int readInt(int index) const;

    // Read a single integer from a buffer texture at (x, y)
    int readInt2D(int x, int y) const;

    // Sample RGBA pixel at texcoord (u, v), nearest filtering
    uint32_t sampleNearest(float u, float v) const;

    // Sample RGBA pixel at integer position (x, y), clamped
    uint32_t samplePixel(int x, int y) const;
};

// ============================================================================
// Draw context — assembled from GL state for each draw call
// ============================================================================
struct DrawContext
{
    CpuShaderType shaderType = CpuShaderType::UNKNOWN;

    // Render target
    uint32_t* targetPixels = nullptr;
    float*    targetDepth  = nullptr;
    int       targetWidth  = 0;
    int       targetHeight = 0;

    // Viewport
    int vpX = 0, vpY = 0, vpW = 0, vpH = 0;

    // Scissor
    bool scissorEnabled = false;
    int  scX = 0, scY = 0, scW = 0, scH = 0;

    // Depth
    bool   depthTestEnabled  = false;
    int    depthFunc         = 0x0201;  // GL_LESS
    bool   depthWriteEnabled = true;

    // Blending
    bool blendEnabled = false;
    int  blendSrcRGB = 1;   // GL_ONE
    int  blendDstRGB = 0;   // GL_ZERO
    int  blendSrcA   = 1;
    int  blendDstA   = 0;

    // Uniforms by name
    std::unordered_map<std::string, std::vector<float>> floatUniforms;
    std::unordered_map<std::string, std::vector<int>>   intUniforms;

    // Bound textures (up to 8 units)
    ShaderTexture textures[8];

    // Uniform accessors
    float getFloat(const std::string& name, int idx = 0) const;
    int   getInt(const std::string& name, int idx = 0) const;
    bool  hasUniform(const std::string& name) const;

    // Get Transform as (x, y, w, h)
    void getTransform(float& tx, float& ty, float& tw, float& th) const;

    // Compute screen-space rect from Transform uniform
    void transformToScreenRect(int& x0, int& y0, int& x1, int& y1) const;
};

// ============================================================================
// Shader identification — call at glLinkProgram time
// ============================================================================
CpuShaderType identifyShader(
    const std::unordered_map<std::string, int>& uniformNames,
    const std::unordered_map<std::string, int>& attribNames,
    const std::string& fragmentSource);

// ============================================================================
// Shader dispatch — call at glDrawArrays time
// Returns true if handled, false for generic fallback
// ============================================================================
bool dispatch(const DrawContext& ctx);

} // namespace wiiu_shaders
