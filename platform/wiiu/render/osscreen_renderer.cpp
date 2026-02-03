#include "osscreen_renderer.h"

#if defined(PLATFORM_WIIU) || defined(__WIIU__)

#include <coreinit/screen.h>
#include <coreinit/cache.h>
#include <malloc.h>
#include <cstring>

wiiu::OSScreenRenderer::~OSScreenRenderer() {
    shutdown();
}

bool wiiu::OSScreenRenderer::init() {
    if (mInitialized)
        return true;

    OSScreenInit();

    // Use standard resolutions since there's no get size function
    mTVWidth = 1280;
    mTVHeight = 720;
    mDRCWidth = 854;
    mDRCHeight = 480;

    mTVSize = OSScreenGetBufferSizeEx(SCREEN_TV);
    mDRCSize = OSScreenGetBufferSizeEx(SCREEN_DRC);

    mTVBuffer = memalign(0x100, mTVSize);
    mDRCBuffer = memalign(0x100, mDRCSize);

    if (!mTVBuffer || !mDRCBuffer) {
        shutdown();
        return false;
    }

    OSScreenSetBufferEx(SCREEN_TV, mTVBuffer);
    OSScreenSetBufferEx(SCREEN_DRC, mDRCBuffer);

    mTVStridePixels = mTVWidth;
    mDRCStridePixels = mDRCWidth;

    OSScreenClearBufferEx(SCREEN_TV, 0x00000000);
    OSScreenClearBufferEx(SCREEN_DRC, 0x00000000);
    OSScreenFlipBuffersEx(SCREEN_TV);
    OSScreenFlipBuffersEx(SCREEN_DRC);

    mInitialized = true;
    return true;
}

void wiiu::OSScreenRenderer::shutdown() {
    if (mTVBuffer) {
        free(mTVBuffer);
        mTVBuffer = nullptr;
    }
    if (mDRCBuffer) {
        free(mDRCBuffer);
        mDRCBuffer = nullptr;
    }
    mTVSize = 0;
    mDRCSize = 0;
    mInitialized = false;
}

void wiiu::OSScreenRenderer::blitToOSScreen(
    const void* src,
    int srcWidth,
    int srcHeight,
    int srcPitch,
    void* dst,
    int dstWidth,
    int dstHeight,
    int dstStridePixels
) {
    if (!src || !dst || srcWidth <= 0 || srcHeight <= 0 || dstWidth <= 0 || dstHeight <= 0)
        return;

    const unsigned char* srcBytes = static_cast<const unsigned char*>(src);
    uint32_t* dstPixels = static_cast<uint32_t*>(dst);

    for (int y = 0; y < dstHeight; ++y) {
        int sy = (y * srcHeight) / dstHeight;
        const uint32_t* srcRow = reinterpret_cast<const uint32_t*>(srcBytes + sy * static_cast<size_t>(srcPitch));
        uint32_t* dstRow = dstPixels + y * static_cast<size_t>(dstStridePixels);
        for (int x = 0; x < dstWidth; ++x) {
            int sx = (x * srcWidth) / dstWidth;
            dstRow[x] = srcRow[sx];
        }
    }
}

void wiiu::OSScreenRenderer::beginFrame() {
}

void wiiu::OSScreenRenderer::uploadFrameBuffer(
    const void* pixels,
    int width,
    int height,
    int pitch
) {
    if (!mInitialized || !pixels || !mTVBuffer || !mDRCBuffer)
        return;
    blitToOSScreen(
        pixels, width, height, pitch,
        mTVBuffer, mTVWidth, mTVHeight, mTVStridePixels
    );
    blitToOSScreen(
        pixels, width, height, pitch,
        mDRCBuffer, mDRCWidth, mDRCHeight, mDRCStridePixels
    );
}

void wiiu::OSScreenRenderer::present() {
    if (!mInitialized)
        return;
    DCFlushRange(mTVBuffer, mTVSize);
    DCFlushRange(mDRCBuffer, mDRCSize);
    OSScreenFlipBuffersEx(SCREEN_TV);
    OSScreenFlipBuffersEx(SCREEN_DRC);
}

#else

wiiu::OSScreenRenderer::~OSScreenRenderer() {}
bool wiiu::OSScreenRenderer::init() { return false; }
void wiiu::OSScreenRenderer::shutdown() {}
void wiiu::OSScreenRenderer::beginFrame() {}
void wiiu::OSScreenRenderer::uploadFrameBuffer(const void*, int, int, int) {}
void wiiu::OSScreenRenderer::present() {}
void wiiu::OSScreenRenderer::blitToOSScreen(const void*, int, int, int, void*, int, int, int) {}

#endif
