/*
 * Lightweight GX2Renderer wrapper
 * - Provides a small, engine-facing API for init/present/quad draws
 * - Currently implemented with WHB/GX2 shim helpers so it builds
 * - Replace internals with real GX2/WUT code when available
 */
#pragma once
#include <cstdint>
#include "../opengl/Texture.h"

namespace rmx {

class GX2Renderer
{
public:
    static GX2Renderer& instance();

    bool initialize(int width, int height);
    void shutdown();

    void beginFrame();
    void endFrame();

    // Simple textured quad draw (texture handle 0 = no texture / solid color)
    void drawTexturedQuad(int x, int y, int w, int h, int texHandle, uint32_t color);

    void enableScissor(bool enable);
    void setScissor(int x, int y, int w, int h);

private:
    GX2Renderer();
    ~GX2Renderer();

    bool mInitialized = false;
    int mWidth = 0;
    int mHeight = 0;
};

} // namespace rmx
