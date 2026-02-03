#pragma once

#include "renderer.h"
#include <cstddef>

/**
 * OSScreen fallback path: guaranteed output if GX2 fails.
 * Fixed resolution 1280x720. No scaling logic inside engine.
 * Converts engine framebuffer (RGBA8888) to OSScreen buffer, flush + swap.
 */
namespace wiiu {

class OSScreenRenderer : public Renderer {
public:
    OSScreenRenderer() = default;
    ~OSScreenRenderer() override;

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
    static const int kTVWidth = 1280;
    static const int kTVHeight = 720;

    void blitToOSScreen(
        const void* src,
        int srcWidth,
        int srcHeight,
        int srcPitch,
        void* dst,
        int dstWidth,
        int dstHeight,
        int dstStridePixels
    );

    bool mInitialized = false;
    void* mTVBuffer = nullptr;
    void* mDRCBuffer = nullptr;
    std::size_t mTVSize = 0;
    std::size_t mDRCSize = 0;
    int mTVWidth = 0;
    int mTVHeight = 0;
    int mDRCWidth = 0;
    int mDRCHeight = 0;
    int mTVStridePixels = 0;
    int mDRCStridePixels = 0;
};

}
