/*
 * CPU-fallback shader implementations for Wii U
 *
 * Each engine GLSL shader is replicated here as a C++ function that
 * processes pixels on the CPU.  The dispatch() entry-point is called
 * from gl_compat's glDrawArrays; it reads the DrawContext (uniforms,
 * textures, render target, depth/blend state) and rasterises the
 * unit-quad geometry that every engine shader uses.
 */

#include "wiiu_shaders.h"

#include <cmath>
#include <cstring>
#include <algorithm>

namespace wiiu_shaders {

// ============================================================================
// Helpers
// ============================================================================
static inline uint8_t clamp8(float v)
{
    int i = static_cast<int>(v * 255.0f + 0.5f);
    return static_cast<uint8_t>(std::clamp(i, 0, 255));
}

static inline uint8_t clamp8i(int v)
{
    return static_cast<uint8_t>(std::clamp(v, 0, 255));
}

static inline uint32_t packRGBA(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    return (uint32_t(a) << 24) | (uint32_t(b) << 16) | (uint32_t(g) << 8) | uint32_t(r);
}

static inline uint32_t packRGBAf(float r, float g, float b, float a)
{
    return packRGBA(clamp8(r), clamp8(g), clamp8(b), clamp8(a));
}

static inline void unpackRGBA(uint32_t c, float& r, float& g, float& b, float& a)
{
    r = float(c & 0xFF) / 255.0f;
    g = float((c >> 8) & 0xFF) / 255.0f;
    b = float((c >> 16) & 0xFF) / 255.0f;
    a = float((c >> 24) & 0xFF) / 255.0f;
}

static inline void unpackRGBAi(uint32_t c, int& r, int& g, int& b, int& a)
{
    r = c & 0xFF;
    g = (c >> 8) & 0xFF;
    b = (c >> 16) & 0xFF;
    a = (c >> 24) & 0xFF;
}

// ============================================================================
// ShaderTexture methods
// ============================================================================
int ShaderTexture::readInt(int index) const
{
    if (!rawBytes || index < 0) return 0;
    if (bytesPerPixel == 1) {
        if (index >= width * height) return 0;
        return rawBytes[index];
    } else {  // 2 bytes per pixel
        int byteIdx = index * 2;
        if (byteIdx + 1 >= width * height * 2) return 0;
        // little-endian read; but on Wii U (big-endian) buffer data
        // may already be big-endian — use big-endian read for safety
        return static_cast<int16_t>((rawBytes[byteIdx] << 8) | rawBytes[byteIdx + 1]);
    }
}

int ShaderTexture::readInt2D(int x, int y) const
{
    if (x < 0 || y < 0 || x >= width || y >= height) return 0;
    return readInt(y * width + x);
}

uint32_t ShaderTexture::sampleNearest(float u, float v) const
{
    if (!pixels || width <= 0 || height <= 0) return 0;
    int tx = static_cast<int>(u * (width - 1) + 0.5f);
    int ty = static_cast<int>(v * (height - 1) + 0.5f);
    tx = std::clamp(tx, 0, width - 1);
    ty = std::clamp(ty, 0, height - 1);
    return pixels[ty * width + tx];
}

uint32_t ShaderTexture::samplePixel(int x, int y) const
{
    if (!pixels || width <= 0 || height <= 0) return 0;
    x = std::clamp(x, 0, width - 1);
    y = std::clamp(y, 0, height - 1);
    return pixels[y * width + x];
}

// ============================================================================
// DrawContext methods
// ============================================================================
float DrawContext::getFloat(const std::string& name, int idx) const
{
    auto it = floatUniforms.find(name);
    if (it == floatUniforms.end() || idx < 0 || idx >= (int)it->second.size()) return 0.0f;
    return it->second[idx];
}

int DrawContext::getInt(const std::string& name, int idx) const
{
    auto it = intUniforms.find(name);
    if (it == intUniforms.end() || idx < 0 || idx >= (int)it->second.size()) return 0;
    return it->second[idx];
}

bool DrawContext::hasUniform(const std::string& name) const
{
    return floatUniforms.count(name) || intUniforms.count(name);
}

void DrawContext::getTransform(float& tx, float& ty, float& tw, float& th) const
{
    tx = getFloat("Transform", 0);
    ty = getFloat("Transform", 1);
    tw = getFloat("Transform", 2);
    th = getFloat("Transform", 3);
}

void DrawContext::transformToScreenRect(int& x0, int& y0, int& x1, int& y1) const
{
    float tx, ty, tw, th;
    getTransform(tx, ty, tw, th);
    x0 = static_cast<int>(tx * vpW) + vpX;
    x1 = static_cast<int>((tx + tw) * vpW) + vpX;
    // Y is flipped: position.y=0 is at screen bottom (large py), position.y=1 is at screen top
    y0 = static_cast<int>((1.0f - ty - th) * vpH) + vpY;
    y1 = static_cast<int>((1.0f - ty) * vpH) + vpY;
}

// ============================================================================
// Shader name for debug logging
// ============================================================================
const char* shaderTypeName(CpuShaderType t)
{
    switch (t) {
        case CpuShaderType::SIMPLE_COPY_SCREEN:    return "CopyScreen";
        case CpuShaderType::SIMPLE_RECT_COLORED:   return "RectColored";
        case CpuShaderType::SIMPLE_RECT_INDEXED:   return "RectIndexed";
        case CpuShaderType::SIMPLE_RECT_TEXTURED:  return "RectTextured";
        case CpuShaderType::SIMPLE_RECT_TEXTURED_UV: return "RectTexturedUV";
        case CpuShaderType::SIMPLE_RECT_VERTEX_COLOR: return "RectVertexColor";
        case CpuShaderType::SIMPLE_RECT_OVERDRAW:  return "RectOverdraw";
        case CpuShaderType::POST_FX_BLUR:          return "PostFXBlur";
        case CpuShaderType::RENDER_PLANE:          return "Plane";
        case CpuShaderType::RENDER_PLANE_HSCROLL:  return "PlaneHScroll";
        case CpuShaderType::RENDER_PLANE_VSCROLL:  return "PlaneVScroll";
        case CpuShaderType::RENDER_PLANE_NO_REPEAT: return "PlaneNoRepeat";
        case CpuShaderType::RENDER_VDP_SPRITE:     return "VdpSprite";
        case CpuShaderType::RENDER_PALETTE_SPRITE: return "PaletteSprite";
        case CpuShaderType::RENDER_COMPONENT_SPRITE: return "ComponentSprite";
        case CpuShaderType::DEBUG_DRAW_PLANE:      return "DebugPlane";
        default: return "Unknown";
    }
}

// ============================================================================
// Shader identification
// ============================================================================
CpuShaderType identifyShader(
    const std::unordered_map<std::string, int>& uniformNames,
    const std::unordered_map<std::string, int>& attribNames,
    const std::string& fragSource)
{
    auto hasU = [&](const char* n) { return uniformNames.count(n) > 0; };
    auto hasA = [&](const char* n) { return attribNames.count(n) > 0; };
    auto srcContains = [&](const char* s) { return fragSource.find(s) != std::string::npos; };

    // Plane shaders: have PatternCacheTexture + IndexTexture
    if (hasU("PatternCacheTexture") && hasU("IndexTexture")) {
        if (srcContains("DebugDrawPlane") || srcContains("debugDraw"))
            return CpuShaderType::DEBUG_DRAW_PLANE;
        if (srcContains("HScrollOffsetsTexture") || hasU("HScrollOffsetsTexture"))
            return CpuShaderType::RENDER_PLANE_HSCROLL;
        if (srcContains("VScrollOffsetsTexture") || hasU("VScrollOffsetsTexture"))
            return CpuShaderType::RENDER_PLANE_VSCROLL;
        if (srcContains("PS_NO_REPEAT") || srcContains("noRepeat"))
            return CpuShaderType::RENDER_PLANE_NO_REPEAT;
        return CpuShaderType::RENDER_PLANE;
    }

    // VDP sprite shader: has PatternCacheTexture + FirstPattern
    if (hasU("PatternCacheTexture") && hasU("FirstPattern"))
        return CpuShaderType::RENDER_VDP_SPRITE;

    // Palette sprite: has PivotOffset + PaletteTexture
    if (hasU("PivotOffset") && hasU("PaletteTexture"))
        return CpuShaderType::RENDER_PALETTE_SPRITE;

    // Component sprite: has PivotOffset without PaletteTexture
    if (hasU("PivotOffset") && !hasU("PaletteTexture"))
        return CpuShaderType::RENDER_COMPONENT_SPRITE;

    // Post-FX blur: has TexelOffset + Kernel
    if (hasU("TexelOffset") && hasU("Kernel"))
        return CpuShaderType::POST_FX_BLUR;

    // Indexed rect: has MainTexture + PaletteTexture + Size
    if (hasU("MainTexture") && hasU("PaletteTexture") && hasU("Size"))
        return CpuShaderType::SIMPLE_RECT_INDEXED;

    // Colored rect: has Color uniform but no MainTexture
    if (hasU("Color") && !hasU("MainTexture"))
        return CpuShaderType::SIMPLE_RECT_COLORED;

    // Overdraw rect: has MainTexture + Transform but source mentions overdraw
    if (hasU("MainTexture") && hasU("Transform") &&
        (srcContains("Overdraw") || srcContains("overdraw") ||
         srcContains("gl_FragColor = vec4(c.rgb, 1.0)")))
        return CpuShaderType::SIMPLE_RECT_OVERDRAW;

    // Textured rect with UV attrib
    if (hasU("MainTexture") && hasA("TexCoord") && hasU("Transform"))
        return CpuShaderType::SIMPLE_RECT_TEXTURED_UV;

    // Vertex color rect
    if (hasA("VertexColor") || hasA("Color"))
        return CpuShaderType::SIMPLE_RECT_VERTEX_COLOR;

    // Textured rect (with Transform)
    if (hasU("MainTexture") && hasU("Transform"))
        return CpuShaderType::SIMPLE_RECT_TEXTURED;

    // Copy screen (MainTexture, no Transform)
    if (hasU("MainTexture") && !hasU("Transform"))
        return CpuShaderType::SIMPLE_COPY_SCREEN;

    return CpuShaderType::UNKNOWN;
}

// ============================================================================
// Blend helpers
// ============================================================================
static inline float blendFactor(int factor, float srcR, float srcA, float dstR, float dstA)
{
    switch (factor) {
        case 0:      return 0.0f;                        // GL_ZERO
        case 1:      return 1.0f;                        // GL_ONE
        case 0x0302: return srcA;                        // GL_SRC_ALPHA
        case 0x0303: return 1.0f - srcA;                 // GL_ONE_MINUS_SRC_ALPHA
        case 0x0304: return dstA;                        // GL_DST_ALPHA
        case 0x0305: return 1.0f - dstA;                 // GL_ONE_MINUS_DST_ALPHA
        case 0x0300: return srcR;                        // GL_SRC_COLOR (approx)
        case 0x0301: return 1.0f - srcR;                 // GL_ONE_MINUS_SRC_COLOR
        default:     return 1.0f;
    }
}

static uint32_t applyBlend(uint32_t src, uint32_t dst,
                           int srcFactorRGB, int dstFactorRGB,
                           int srcFactorA, int dstFactorA)
{
    float sr, sg, sb, sa, dr, dg, db, da;
    unpackRGBA(src, sr, sg, sb, sa);
    unpackRGBA(dst, dr, dg, db, da);

    float sfR = blendFactor(srcFactorRGB, sr, sa, dr, da);
    float dfR = blendFactor(dstFactorRGB, sr, sa, dr, da);
    float sfA = blendFactor(srcFactorA,   sa, sa, da, da);
    float dfA = blendFactor(dstFactorA,   sa, sa, da, da);

    float outR = std::clamp(sr * sfR + dr * dfR, 0.0f, 1.0f);
    float outG = std::clamp(sg * sfR + dg * dfR, 0.0f, 1.0f);
    float outB = std::clamp(sb * sfR + db * dfR, 0.0f, 1.0f);
    float outA = std::clamp(sa * sfA + da * dfA, 0.0f, 1.0f);

    return packRGBAf(outR, outG, outB, outA);
}

// ============================================================================
// Depth test
// ============================================================================
static bool passesDepthTest(int func, float fragZ, float bufZ)
{
    switch (func) {
        case 0x0200: return false;              // GL_NEVER
        case 0x0201: return fragZ < bufZ;       // GL_LESS
        case 0x0202: return fragZ == bufZ;      // GL_EQUAL
        case 0x0203: return fragZ <= bufZ;      // GL_LEQUAL
        case 0x0204: return fragZ > bufZ;       // GL_GREATER
        case 0x0205: return fragZ != bufZ;      // GL_NOTEQUAL
        case 0x0206: return fragZ >= bufZ;      // GL_GEQUAL
        case 0x0207: return true;               // GL_ALWAYS
        default:     return true;
    }
}

// ============================================================================
// Write a pixel with depth test and blending
// ============================================================================
static inline void writePixel(const DrawContext& ctx, int px, int py,
                               uint32_t color, float depth)
{
    if (px < 0 || py < 0 || px >= ctx.targetWidth || py >= ctx.targetHeight)
        return;

    // Scissor test
    if (ctx.scissorEnabled) {
        // Scissor Y is in OpenGL convention (bottom-up); convert
        int scTop = ctx.targetHeight - ctx.scY - ctx.scH;
        int scBot = ctx.targetHeight - ctx.scY;
        if (px < ctx.scX || px >= ctx.scX + ctx.scW ||
            py < scTop || py >= scBot)
            return;
    }

    int idx = py * ctx.targetWidth + px;

    // Depth test
    if (ctx.depthTestEnabled && ctx.targetDepth) {
        if (!passesDepthTest(ctx.depthFunc, depth, ctx.targetDepth[idx]))
            return;
    }

    // Blending
    uint32_t finalColor = color;
    if (ctx.blendEnabled) {
        finalColor = applyBlend(color, ctx.targetPixels[idx],
                                ctx.blendSrcRGB, ctx.blendDstRGB,
                                ctx.blendSrcA, ctx.blendDstA);
    }

    ctx.targetPixels[idx] = finalColor;

    // Depth write
    if (ctx.depthWriteEnabled && ctx.targetDepth)
        ctx.targetDepth[idx] = depth;
}

// ============================================================================
// Individual shader implementations
// ============================================================================

// --- SimpleCopyScreen ---
// Full-screen texture blit
static bool shader_CopyScreen(const DrawContext& ctx)
{
    const auto& tex = ctx.textures[0];
    if (!tex.pixels) return false;

    for (int py = 0; py < ctx.vpH; ++py) {
        int dstY = py + ctx.vpY;
        float v = float(py) / float(ctx.vpH);
        for (int px = 0; px < ctx.vpW; ++px) {
            int dstX = px + ctx.vpX;
            float u = float(px) / float(ctx.vpW);
            uint32_t c = tex.sampleNearest(u, v);
            writePixel(ctx, dstX, dstY, c, 0.5f);
        }
    }
    return true;
}

// --- SimpleRectColored ---
// Solid-color rectangle
static bool shader_RectColored(const DrawContext& ctx)
{
    float r = ctx.getFloat("Color", 0);
    float g = ctx.getFloat("Color", 1);
    float b = ctx.getFloat("Color", 2);
    float a = ctx.getFloat("Color", 3);
    uint32_t color = packRGBAf(r, g, b, a);

    int x0, y0, x1, y1;
    ctx.transformToScreenRect(x0, y0, x1, y1);
    x0 = std::max(x0, 0); y0 = std::max(y0, 0);
    x1 = std::min(x1, ctx.targetWidth);
    y1 = std::min(y1, ctx.targetHeight);

    for (int py = y0; py < y1; ++py)
        for (int px = x0; px < x1; ++px)
            writePixel(ctx, px, py, color, 0.5f);
    return true;
}

// --- SimpleRectTextured ---
// Textured rectangle with optional TintColor and AddedColor
static bool shader_RectTextured(const DrawContext& ctx)
{
    const auto& tex = ctx.textures[0];
    if (!tex.pixels) return false;

    float tintR = 1.0f, tintG = 1.0f, tintB = 1.0f, tintA = 1.0f;
    float addR = 0.0f, addG = 0.0f, addB = 0.0f, addA = 0.0f;
    if (ctx.hasUniform("TintColor")) {
        tintR = ctx.getFloat("TintColor", 0);
        tintG = ctx.getFloat("TintColor", 1);
        tintB = ctx.getFloat("TintColor", 2);
        tintA = ctx.getFloat("TintColor", 3);
    }
    if (ctx.hasUniform("AddedColor")) {
        addR = ctx.getFloat("AddedColor", 0);
        addG = ctx.getFloat("AddedColor", 1);
        addB = ctx.getFloat("AddedColor", 2);
        addA = ctx.getFloat("AddedColor", 3);
    }

    int x0, y0, x1, y1;
    ctx.transformToScreenRect(x0, y0, x1, y1);
    x0 = std::max(x0, 0); y0 = std::max(y0, 0);
    x1 = std::min(x1, ctx.targetWidth);
    y1 = std::min(y1, ctx.targetHeight);

    float rw = (x1 > x0) ? 1.0f / float(x1 - x0) : 0.0f;
    float rh = (y1 > y0) ? 1.0f / float(y1 - y0) : 0.0f;

    for (int py = y0; py < y1; ++py) {
        float v = float(y1 - 1 - py) * rh;  // flip V: top of rect = v near 1
        // Actually: position.y goes 0→1 from bottom to top.
        // screen y increases downward, so py=y0 is top (position.y=1).
        v = float(py - y0) * rh;
        // Correction: The vertex shader maps position.y=0 → bottom (large py=y1)
        //             position.y=1 → top (small py=y0)
        // So v = (y1 - py) / (y1 - y0)  (v=1 at py=y0, v=0 at py=y1-1)
        // But texcoord = position, and texture v=0 is bottom.
        // For screen textures, v=0 should map to the top of the source.
        // Let's use standard mapping: v = (py - y0) * rh
        // This maps py=y0 → v=0 (top of texture in screen space).
        for (int px = x0; px < x1; ++px) {
            float u = float(px - x0) * rw;

            uint32_t c = tex.sampleNearest(u, v);
            float sr, sg, sb, sa;
            unpackRGBA(c, sr, sg, sb, sa);

            sr = sr * tintR + addR;
            sg = sg * tintG + addG;
            sb = sb * tintB + addB;
            sa = sa * tintA + addA;

            writePixel(ctx, px, py, packRGBAf(sr, sg, sb, sa), 0.5f);
        }
    }
    return true;
}

// --- SimpleRectIndexed ---
// Palette-indexed texture: read index from buffer/texture, look up in palette
static bool shader_RectIndexed(const DrawContext& ctx)
{
    // MainTexture = texture unit 0, PaletteTexture = texture unit 1
    const auto& mainTex = ctx.textures[0];
    const auto& palTex = ctx.textures[1];
    if (!palTex.pixels) return false;

    int sizeW = ctx.getInt("Size", 0);
    int sizeH = ctx.getInt("Size", 1);
    if (sizeW <= 0) sizeW = mainTex.width;
    if (sizeH <= 0) sizeH = mainTex.height;

    float tintR = 1.0f, tintG = 1.0f, tintB = 1.0f, tintA = 1.0f;
    float addR = 0.0f, addG = 0.0f, addB = 0.0f, addA = 0.0f;
    if (ctx.hasUniform("TintColor")) {
        tintR = ctx.getFloat("TintColor", 0);
        tintG = ctx.getFloat("TintColor", 1);
        tintB = ctx.getFloat("TintColor", 2);
        tintA = ctx.getFloat("TintColor", 3);
    }
    if (ctx.hasUniform("AddedColor")) {
        addR = ctx.getFloat("AddedColor", 0);
        addG = ctx.getFloat("AddedColor", 1);
        addB = ctx.getFloat("AddedColor", 2);
        addA = ctx.getFloat("AddedColor", 3);
    }

    int x0, y0, x1, y1;
    ctx.transformToScreenRect(x0, y0, x1, y1);
    x0 = std::max(x0, 0); y0 = std::max(y0, 0);
    x1 = std::min(x1, ctx.targetWidth);
    y1 = std::min(y1, ctx.targetHeight);

    float rw = (x1 > x0) ? 1.0f / float(x1 - x0) : 0.0f;
    float rh = (y1 > y0) ? 1.0f / float(y1 - y0) : 0.0f;

    for (int py = y0; py < y1; ++py) {
        float v = float(py - y0) * rh;
        for (int px = x0; px < x1; ++px) {
            float u = float(px - x0) * rw;

            int texX = static_cast<int>(u * sizeW);
            int texY = static_cast<int>(v * sizeH);
            texX = std::clamp(texX, 0, sizeW - 1);
            texY = std::clamp(texY, 0, sizeH - 1);

            int index = 0;
            if (mainTex.isBufferTexture && mainTex.rawBytes) {
                index = mainTex.readInt(texY * sizeW + texX);
            } else if (mainTex.pixels) {
                uint32_t raw = mainTex.samplePixel(texX, texY);
                index = raw & 0xFF;  // Red channel
            }

            if (index == 0) continue;  // discard transparent

            // Palette lookup: row 0, column = index
            uint32_t palColor = palTex.samplePixel(index, 0);
            float pr, pg, pb, pa;
            unpackRGBA(palColor, pr, pg, pb, pa);

            pr = pr * tintR + addR;
            pg = pg * tintG + addG;
            pb = pb * tintB + addB;
            pa = pa * tintA + addA;

            writePixel(ctx, px, py, packRGBAf(pr, pg, pb, pa), 0.5f);
        }
    }
    return true;
}

// --- SimpleRectOverdraw ---
// Same as textured rect but forces alpha = 1
static bool shader_RectOverdraw(const DrawContext& ctx)
{
    const auto& tex = ctx.textures[0];
    if (!tex.pixels) return false;

    int x0, y0, x1, y1;
    ctx.transformToScreenRect(x0, y0, x1, y1);
    x0 = std::max(x0, 0); y0 = std::max(y0, 0);
    x1 = std::min(x1, ctx.targetWidth);
    y1 = std::min(y1, ctx.targetHeight);

    float rw = (x1 > x0) ? 1.0f / float(x1 - x0) : 0.0f;
    float rh = (y1 > y0) ? 1.0f / float(y1 - y0) : 0.0f;

    for (int py = y0; py < y1; ++py) {
        float v = float(py - y0) * rh;
        for (int px = x0; px < x1; ++px) {
            float u = float(px - x0) * rw;
            uint32_t c = tex.sampleNearest(u, v);
            c |= 0xFF000000u;  // force alpha = 1
            writePixel(ctx, px, py, c, 0.5f);
        }
    }
    return true;
}

// --- PostFXBlur ---
// 3x3 Gaussian kernel applied to a texture
static bool shader_PostFXBlur(const DrawContext& ctx)
{
    const auto& tex = ctx.textures[0];
    if (!tex.pixels) return false;

    float texOffX = ctx.getFloat("TexelOffset", 0);
    float texOffY = ctx.getFloat("TexelOffset", 1);

    // Kernel is 3 floats for a separable or 3x3 kernel
    float k0 = ctx.getFloat("Kernel", 0);
    float k1 = ctx.getFloat("Kernel", 1);
    float k2 = ctx.getFloat("Kernel", 2);
    if (k0 == 0.0f && k1 == 0.0f && k2 == 0.0f) {
        k0 = 0.25f; k1 = 0.5f; k2 = 0.25f;  // default Gaussian
    }

    for (int py = 0; py < ctx.vpH; ++py) {
        int dstY = py + ctx.vpY;
        float v = float(py) / float(ctx.vpH);
        for (int px = 0; px < ctx.vpW; ++px) {
            int dstX = px + ctx.vpX;
            float u = float(px) / float(ctx.vpW);

            float accR = 0, accG = 0, accB = 0, accA = 0;
            float weights[3] = {k0, k1, k2};

            for (int dy = -1; dy <= 1; ++dy) {
                for (int dx = -1; dx <= 1; ++dx) {
                    float su = u + float(dx) * texOffX;
                    float sv = v + float(dy) * texOffY;
                    uint32_t s = tex.sampleNearest(su, sv);
                    float sr, sg, sb, sa;
                    unpackRGBA(s, sr, sg, sb, sa);
                    float w = weights[dx + 1] * weights[dy + 1];
                    accR += sr * w;
                    accG += sg * w;
                    accB += sb * w;
                    accA += sa * w;
                }
            }

            writePixel(ctx, dstX, dstY, packRGBAf(accR, accG, accB, accA), 0.5f);
        }
    }
    return true;
}

// ============================================================================
// Plane shader common logic
// ============================================================================
static bool shader_Plane(const DrawContext& ctx, bool hScroll, bool vScroll, bool noRepeat)
{
    // Texture units:
    // 0 = PatternCacheTexture (buffer, R8UI — 8x8 tile pixel indices)
    // 1 = PaletteTexture (2D RGBA, 256 wide)
    // 2 = IndexTexture (buffer, R16I — plane tile map entries)
    // 3 = HScrollOffsetsTexture (buffer, R16I — per-line H-scroll)
    // 4 = VScrollOffsetsTexture (buffer, R16I — per-column V-scroll)
    const auto& patternCache = ctx.textures[0];
    const auto& palette      = ctx.textures[1];
    const auto& indexTex     = ctx.textures[2];
    const auto& hScrollTex   = ctx.textures[3];
    const auto& vScrollTex   = ctx.textures[4];

    if (!palette.pixels) return false;

    int playfieldW = ctx.getInt("PlayfieldSize", 0);
    int playfieldH = ctx.getInt("PlayfieldSize", 1);
    if (playfieldW <= 0) playfieldW = 64;
    if (playfieldH <= 0) playfieldH = 32;

    int gameResW = ctx.getInt("GameResolution", 0);
    int gameResH = ctx.getInt("GameResolution", 1);
    if (gameResW <= 0) gameResW = ctx.vpW;
    if (gameResH <= 0) gameResH = ctx.vpH;

    int priorityFlag = ctx.getInt("PriorityFlag", 0);

    int scrollOffsetX = ctx.getInt("ScrollOffsetX", 0);
    int scrollOffsetY = ctx.getInt("ScrollOffsetY", 0);

    int paletteOffset = ctx.getInt("PaletteOffset", 0);

    // Depth value for priority planes
    float depthVal = (priorityFlag != 0) ? 0.5f : 1.0f;

    int x0, y0, x1, y1;
    ctx.transformToScreenRect(x0, y0, x1, y1);
    x0 = std::max(x0, 0); y0 = std::max(y0, 0);
    x1 = std::min(x1, ctx.targetWidth);
    y1 = std::min(y1, ctx.targetHeight);

    int planePixelW = playfieldW * 8;
    int planePixelH = playfieldH * 8;

    for (int py = y0; py < y1; ++py) {
        // gl_FragCoord.y in OpenGL = bottom-up
        int fragY = (ctx.vpH - 1) - (py - ctx.vpY);

        // Per-line horizontal scroll offset
        int hOff = scrollOffsetX;
        if (hScroll && hScrollTex.rawBytes) {
            hOff += hScrollTex.readInt(fragY);
        }

        for (int px = x0; px < x1; ++px) {
            int fragX = px - ctx.vpX;

            // Per-column vertical scroll offset
            int vOff = scrollOffsetY;
            if (vScroll && vScrollTex.rawBytes) {
                int col = fragX >> 4;  // 16-pixel columns for V-scroll
                vOff += vScrollTex.readInt(col);
            }

            // Pixel position in plane space
            int planeX = fragX + hOff;
            int planeY = fragY + vOff;

            if (noRepeat) {
                if (planeX < 0 || planeX >= planePixelW ||
                    planeY < 0 || planeY >= planePixelH)
                    continue;  // discard
            } else {
                // Wrap around
                planeX = ((planeX % planePixelW) + planePixelW) % planePixelW;
                planeY = ((planeY % planePixelH) + planePixelH) % planePixelH;
            }

            // Tile position
            int tileX = planeX >> 3;  // / 8
            int tileY = planeY >> 3;
            int inTileX = planeX & 7;  // % 8
            int inTileY = planeY & 7;

            // Read tile entry from index texture
            int tileIdx = tileY * playfieldW + tileX;
            int tileEntry = 0;
            if (indexTex.rawBytes) {
                tileEntry = indexTex.readInt(tileIdx);
            }

            // Decode tile entry bits
            int patternIndex = tileEntry & 0x07FF;
            bool flipH = (tileEntry & 0x0800) != 0;
            bool flipV = (tileEntry & 0x1000) != 0;
            int priority = (tileEntry >> 13) & 1;
            int paletteRow = (tileEntry >> 14) & 3;

            // Priority check
            if (priority != priorityFlag)
                continue;  // discard

            // Apply flip
            if (flipH) inTileX = 7 - inTileX;
            if (flipV) inTileY = 7 - inTileY;

            // Read pixel index from pattern cache
            int patternPixelIdx = patternIndex * 64 + inTileY * 8 + inTileX;
            int colorIndex = 0;
            if (patternCache.rawBytes) {
                colorIndex = patternCache.readInt(patternPixelIdx);
            }

            if (colorIndex == 0)
                continue;  // transparent

            // Palette lookup
            int palX = (paletteRow + paletteOffset) * 16 + colorIndex;
            uint32_t palColor = palette.samplePixel(palX, 0);

            writePixel(ctx, px, py, palColor, depthVal);
        }
    }
    return true;
}

// ============================================================================
// VDP Sprite shader
// ============================================================================
static bool shader_VdpSprite(const DrawContext& ctx)
{
    // Tex unit 0 = PatternCacheTexture (buffer, R8UI)
    // Tex unit 1 = PaletteTexture (2D RGBA)
    const auto& patternCache = ctx.textures[0];
    const auto& palette      = ctx.textures[1];

    if (!palette.pixels) return false;

    int firstPattern = ctx.getInt("FirstPattern", 0);
    int gameResW = ctx.getInt("GameResolution", 0);
    int gameResH = ctx.getInt("GameResolution", 1);
    if (gameResW <= 0) gameResW = ctx.vpW;
    if (gameResH <= 0) gameResH = ctx.vpH;

    int paletteOffset = 0;
    if (ctx.hasUniform("PaletteOffset"))
        paletteOffset = ctx.getInt("PaletteOffset", 0);

    float tintR = 1.0f, tintG = 1.0f, tintB = 1.0f, tintA = 1.0f;
    float addR = 0.0f, addG = 0.0f, addB = 0.0f, addA = 0.0f;
    if (ctx.hasUniform("TintColor")) {
        tintR = ctx.getFloat("TintColor", 0);
        tintG = ctx.getFloat("TintColor", 1);
        tintB = ctx.getFloat("TintColor", 2);
        tintA = ctx.getFloat("TintColor", 3);
    }
    if (ctx.hasUniform("AddedColor")) {
        addR = ctx.getFloat("AddedColor", 0);
        addG = ctx.getFloat("AddedColor", 1);
        addB = ctx.getFloat("AddedColor", 2);
        addA = ctx.getFloat("AddedColor", 3);
    }

    int x0, y0, x1, y1;
    ctx.transformToScreenRect(x0, y0, x1, y1);
    x0 = std::max(x0, 0); y0 = std::max(y0, 0);
    x1 = std::min(x1, ctx.targetWidth);
    y1 = std::min(y1, ctx.targetHeight);

    float rw = (x1 > x0) ? 1.0f / float(x1 - x0) : 0.0f;
    float rh = (y1 > y0) ? 1.0f / float(y1 - y0) : 0.0f;

    for (int py = y0; py < y1; ++py) {
        for (int px = x0; px < x1; ++px) {
            float u = float(px - x0) * rw;
            float v = float(py - y0) * rh;

            // Map UV to texture position in the pattern range
            int texW = (int)((x1 - x0) + 0.5f);
            int texH = (int)((y1 - y0) + 0.5f);
            int texX = static_cast<int>(u * texW);
            int texY = static_cast<int>(v * texH);

            // Read color index from pattern cache
            int pixelIndex = firstPattern * 64 + texY * texW + texX;
            int colorIndex = 0;
            if (patternCache.rawBytes) {
                colorIndex = patternCache.readInt(pixelIndex);
            }

            if (colorIndex == 0)
                continue;

            // Palette lookup
            int palX = paletteOffset * 16 + colorIndex;
            uint32_t palColor = palette.samplePixel(palX, 0);
            float pr, pg, pb, pa;
            unpackRGBA(palColor, pr, pg, pb, pa);

            pr = pr * tintR + addR;
            pg = pg * tintG + addG;
            pb = pb * tintB + addB;
            pa = pa * tintA + addA;

            writePixel(ctx, px, py, packRGBAf(pr, pg, pb, pa), 0.0f);
        }
    }
    return true;
}

// ============================================================================
// Palette Sprite shader
// ============================================================================
static bool shader_PaletteSprite(const DrawContext& ctx)
{
    // Tex unit 0 = Texture (bitmap with palette indices)
    // Tex unit 1 = PaletteTexture (2D RGBA)
    const auto& spriteTex = ctx.textures[0];
    const auto& palette   = ctx.textures[1];

    if (!palette.pixels) return false;

    float pivotX = ctx.getFloat("PivotOffset", 0);
    float pivotY = ctx.getFloat("PivotOffset", 1);

    int atexBase = 0;
    if (ctx.hasUniform("Atex"))
        atexBase = ctx.getInt("Atex", 0);

    float tintR = 1.0f, tintG = 1.0f, tintB = 1.0f, tintA = 1.0f;
    float addR = 0.0f, addG = 0.0f, addB = 0.0f, addA = 0.0f;
    if (ctx.hasUniform("TintColor")) {
        tintR = ctx.getFloat("TintColor", 0);
        tintG = ctx.getFloat("TintColor", 1);
        tintB = ctx.getFloat("TintColor", 2);
        tintA = ctx.getFloat("TintColor", 3);
    }
    if (ctx.hasUniform("AddedColor")) {
        addR = ctx.getFloat("AddedColor", 0);
        addG = ctx.getFloat("AddedColor", 1);
        addB = ctx.getFloat("AddedColor", 2);
        addA = ctx.getFloat("AddedColor", 3);
    }

    // Transformation matrix (2x2)
    float m00 = 1.0f, m01 = 0.0f, m10 = 0.0f, m11 = 1.0f;
    if (ctx.hasUniform("TransformationMatrix")) {
        m00 = ctx.getFloat("TransformationMatrix", 0);
        m01 = ctx.getFloat("TransformationMatrix", 1);
        m10 = ctx.getFloat("TransformationMatrix", 2);
        m11 = ctx.getFloat("TransformationMatrix", 3);
    }

    int x0, y0, x1, y1;
    ctx.transformToScreenRect(x0, y0, x1, y1);
    x0 = std::max(x0, 0); y0 = std::max(y0, 0);
    x1 = std::min(x1, ctx.targetWidth);
    y1 = std::min(y1, ctx.targetHeight);

    int sprW = spriteTex.width > 0 ? spriteTex.width : 1;
    int sprH = spriteTex.height > 0 ? spriteTex.height : 1;

    for (int py = y0; py < y1; ++py) {
        for (int px = x0; px < x1; ++px) {
            // Screen position relative to sprite origin
            float sx = float(px) - pivotX;
            float sy = float(py) - pivotY;

            // Apply inverse transform to get sprite-local coords
            float det = m00 * m11 - m01 * m10;
            if (std::fabs(det) < 1e-6f) continue;
            float invDet = 1.0f / det;
            float localX = (m11 * sx - m01 * sy) * invDet;
            float localY = (-m10 * sx + m00 * sy) * invDet;

            int texX = static_cast<int>(localX + 0.5f);
            int texY = static_cast<int>(localY + 0.5f);

            if (texX < 0 || texX >= sprW || texY < 0 || texY >= sprH)
                continue;

            // Read palette index from sprite texture
            int colorIndex = 0;
            if (spriteTex.isBufferTexture && spriteTex.rawBytes) {
                colorIndex = spriteTex.readInt(texY * sprW + texX);
            } else if (spriteTex.pixels) {
                uint32_t raw = spriteTex.samplePixel(texX, texY);
                colorIndex = raw & 0xFF;
            }

            if (colorIndex == 0)
                continue;

            // Apply atex
            int palIndex = atexBase + colorIndex;
            uint32_t palColor = palette.samplePixel(palIndex & 0xFF, 0);
            float pr, pg, pb, pa;
            unpackRGBA(palColor, pr, pg, pb, pa);

            pr = pr * tintR + addR;
            pg = pg * tintG + addG;
            pb = pb * tintB + addB;
            pa = pa * tintA + addA;

            writePixel(ctx, px, py, packRGBAf(pr, pg, pb, pa), 0.0f);
        }
    }
    return true;
}

// ============================================================================
// Component Sprite shader (RGBA, not indexed)
// ============================================================================
static bool shader_ComponentSprite(const DrawContext& ctx)
{
    const auto& spriteTex = ctx.textures[0];
    if (!spriteTex.pixels) return false;

    float pivotX = ctx.getFloat("PivotOffset", 0);
    float pivotY = ctx.getFloat("PivotOffset", 1);

    float tintR = 1.0f, tintG = 1.0f, tintB = 1.0f, tintA = 1.0f;
    float addR = 0.0f, addG = 0.0f, addB = 0.0f, addA = 0.0f;
    if (ctx.hasUniform("TintColor")) {
        tintR = ctx.getFloat("TintColor", 0);
        tintG = ctx.getFloat("TintColor", 1);
        tintB = ctx.getFloat("TintColor", 2);
        tintA = ctx.getFloat("TintColor", 3);
    }
    if (ctx.hasUniform("AddedColor")) {
        addR = ctx.getFloat("AddedColor", 0);
        addG = ctx.getFloat("AddedColor", 1);
        addB = ctx.getFloat("AddedColor", 2);
        addA = ctx.getFloat("AddedColor", 3);
    }

    // 2x2 transformation matrix
    float m00 = 1.0f, m01 = 0.0f, m10 = 0.0f, m11 = 1.0f;
    if (ctx.hasUniform("TransformationMatrix")) {
        m00 = ctx.getFloat("TransformationMatrix", 0);
        m01 = ctx.getFloat("TransformationMatrix", 1);
        m10 = ctx.getFloat("TransformationMatrix", 2);
        m11 = ctx.getFloat("TransformationMatrix", 3);
    }

    int x0, y0, x1, y1;
    ctx.transformToScreenRect(x0, y0, x1, y1);
    x0 = std::max(x0, 0); y0 = std::max(y0, 0);
    x1 = std::min(x1, ctx.targetWidth);
    y1 = std::min(y1, ctx.targetHeight);

    int sprW = spriteTex.width > 0 ? spriteTex.width : 1;
    int sprH = spriteTex.height > 0 ? spriteTex.height : 1;

    for (int py = y0; py < y1; ++py) {
        for (int px = x0; px < x1; ++px) {
            float sx = float(px) - pivotX;
            float sy = float(py) - pivotY;

            float det = m00 * m11 - m01 * m10;
            if (std::fabs(det) < 1e-6f) continue;
            float invDet = 1.0f / det;
            float localX = (m11 * sx - m01 * sy) * invDet;
            float localY = (-m10 * sx + m00 * sy) * invDet;

            int texX = static_cast<int>(localX + 0.5f);
            int texY = static_cast<int>(localY + 0.5f);

            if (texX < 0 || texX >= sprW || texY < 0 || texY >= sprH)
                continue;

            uint32_t c = spriteTex.samplePixel(texX, texY);
            float sr, sg, sb, sa;
            unpackRGBA(c, sr, sg, sb, sa);

            if (sa < 1.0f / 255.0f) continue;  // alpha discard

            sr = sr * tintR + addR;
            sg = sg * tintG + addG;
            sb = sb * tintB + addB;
            sa = sa * tintA + addA;

            writePixel(ctx, px, py, packRGBAf(sr, sg, sb, sa), 0.0f);
        }
    }
    return true;
}

// ============================================================================
// Vertex-color rect
// ============================================================================
static bool shader_VertexColor(const DrawContext& ctx)
{
    // Vertex color shaders are rare; just fill with the first color found
    float r = ctx.getFloat("Color", 0);
    float g = ctx.getFloat("Color", 1);
    float b = ctx.getFloat("Color", 2);
    float a = ctx.getFloat("Color", 3);
    if (r == 0.0f && g == 0.0f && b == 0.0f && a == 0.0f)
        a = 1.0f;  // default white

    int x0, y0, x1, y1;
    ctx.transformToScreenRect(x0, y0, x1, y1);
    x0 = std::max(x0, 0); y0 = std::max(y0, 0);
    x1 = std::min(x1, ctx.targetWidth);
    y1 = std::min(y1, ctx.targetHeight);

    uint32_t color = packRGBAf(r, g, b, a);
    for (int py = y0; py < y1; ++py)
        for (int px = x0; px < x1; ++px)
            writePixel(ctx, px, py, color, 0.5f);
    return true;
}

// ============================================================================
// Dispatch
// ============================================================================
bool dispatch(const DrawContext& ctx)
{
    switch (ctx.shaderType) {
        case CpuShaderType::SIMPLE_COPY_SCREEN:
            return shader_CopyScreen(ctx);
        case CpuShaderType::SIMPLE_RECT_COLORED:
            return shader_RectColored(ctx);
        case CpuShaderType::SIMPLE_RECT_INDEXED:
            return shader_RectIndexed(ctx);
        case CpuShaderType::SIMPLE_RECT_TEXTURED:
        case CpuShaderType::SIMPLE_RECT_TEXTURED_UV:
            return shader_RectTextured(ctx);
        case CpuShaderType::SIMPLE_RECT_VERTEX_COLOR:
            return shader_VertexColor(ctx);
        case CpuShaderType::SIMPLE_RECT_OVERDRAW:
            return shader_RectOverdraw(ctx);
        case CpuShaderType::POST_FX_BLUR:
            return shader_PostFXBlur(ctx);
        case CpuShaderType::RENDER_PLANE:
            return shader_Plane(ctx, false, false, false);
        case CpuShaderType::RENDER_PLANE_HSCROLL:
            return shader_Plane(ctx, true, false, false);
        case CpuShaderType::RENDER_PLANE_VSCROLL:
            return shader_Plane(ctx, false, true, false);
        case CpuShaderType::RENDER_PLANE_NO_REPEAT:
            return shader_Plane(ctx, false, false, true);
        case CpuShaderType::RENDER_VDP_SPRITE:
            return shader_VdpSprite(ctx);
        case CpuShaderType::RENDER_PALETTE_SPRITE:
            return shader_PaletteSprite(ctx);
        case CpuShaderType::RENDER_COMPONENT_SPRITE:
            return shader_ComponentSprite(ctx);
        case CpuShaderType::DEBUG_DRAW_PLANE:
            return shader_Plane(ctx, false, false, false);  // same logic
        default:
            return false;
    }
}

} // namespace wiiu_shaders
