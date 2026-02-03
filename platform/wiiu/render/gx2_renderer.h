#pragma once

#include "renderer.h"

/**
 * GX2 primary path: native GPU rendering.
 * Initializes GX2, allocates GPU texture for engine framebuffer,
 * uploads CPU framebuffer each frame, draws full-screen textured quad, VSync & swap.
 */
namespace wiiu {

class GX2Renderer : public Renderer {
public:
    GX2Renderer() = default;
    ~GX2Renderer() override;

    bool init() override;
    void shutdown() override;

    void beginFrame() override;
    void uploadFrameBuffer(
        const void* pixels,
        int width,
        int height,
        int pitch
    ) override;
    void present() override;

private:
    bool mInitialized = false;
    int mTexWidth = 0;
    int mTexHeight = 0;
    int mTargetWidth = 0;
    int mTargetHeight = 0;
    void* mTextureHandle = nullptr;
};

}
