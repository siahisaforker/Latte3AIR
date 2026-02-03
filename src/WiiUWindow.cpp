#include "WiiUWindow.h"

#if defined(__WIIU__) || defined(PLATFORM_WIIU)
#include <coreinit/screen.h>
#include <coreinit/cache.h>
#include <malloc.h>
#include <cstring>

bool WiiUWindow::initialize(int srcWidth, int srcHeight) {
    (void)srcWidth;
    (void)srcHeight;

    if (mInitialized) return true;

    OSScreenInit();

    OSScreenGetScreenSizeEx(OS_SCREEN_TV, &mTVWidth, &mTVHeight);
    OSScreenGetScreenSizeEx(OS_SCREEN_DRC, &mDRCWidth, &mDRCHeight);

    mTVSize = OSScreenGetBufferSizeEx(OS_SCREEN_TV);
    mDRCSize = OSScreenGetBufferSizeEx(OS_SCREEN_DRC);

    mTVBuffer = memalign(0x100, mTVSize);
    mDRCBuffer = memalign(0x100, mDRCSize);

    if (!mTVBuffer || !mDRCBuffer) {
        shutdown();
        return false;
    }

    OSScreenSetBufferEx(OS_SCREEN_TV, mTVBuffer);
    OSScreenSetBufferEx(OS_SCREEN_DRC, mDRCBuffer);

    mTVStridePixels = mTVWidth;
    mDRCStridePixels = mDRCWidth;

    OSScreenClearBufferEx(OS_SCREEN_TV, 0x00000000);
    OSScreenClearBufferEx(OS_SCREEN_DRC, 0x00000000);
    OSScreenFlipBuffersEx(OS_SCREEN_TV);
    OSScreenFlipBuffersEx(OS_SCREEN_DRC);

    mInitialized = true;
    return true;
}

void WiiUWindow::shutdown() {
    if (mTVBuffer) {
        free(mTVBuffer);
        mTVBuffer = nullptr;
    }
    if (mDRCBuffer) {
        free(mDRCBuffer);
        mDRCBuffer = nullptr;
    }
    mInitialized = false;
}

void WiiUWindow::blitScale(const uint32_t* src, int sw, int sh,
                           uint32_t* dst, int dw, int dh, int dstStridePixels) {
    if (!src || !dst || sw <= 0 || sh <= 0 || dw <= 0 || dh <= 0) return;

    for (int y = 0; y < dh; ++y) {
        int sy = (y * sh) / dh;
        const uint32_t* srcRow = src + sy * sw;
        uint32_t* dstRow = dst + y * dstStridePixels;
        for (int x = 0; x < dw; ++x) {
            int sx = (x * sw) / dw;
            dstRow[x] = srcRow[sx];
        }
    }
}

void WiiUWindow::present(const uint32_t* pixels, int srcWidth, int srcHeight) {
    if (!mInitialized || !pixels) return;

    blitScale(pixels, srcWidth, srcHeight,
              reinterpret_cast<uint32_t*>(mTVBuffer), mTVWidth, mTVHeight, mTVStridePixels);
    blitScale(pixels, srcWidth, srcHeight,
              reinterpret_cast<uint32_t*>(mDRCBuffer), mDRCWidth, mDRCHeight, mDRCStridePixels);

    DCFlushRange(mTVBuffer, mTVSize);
    DCFlushRange(mDRCBuffer, mDRCSize);

    OSScreenFlipBuffersEx(OS_SCREEN_TV);
    OSScreenFlipBuffersEx(OS_SCREEN_DRC);
}

#else

bool WiiUWindow::initialize(int srcWidth, int srcHeight) {
    (void)srcWidth;
    (void)srcHeight;
    mInitialized = true;
    return true;
}

void WiiUWindow::shutdown() {}

void WiiUWindow::blitScale(const uint32_t* src, int sw, int sh,
                           uint32_t* dst, int dw, int dh, int dstStridePixels) {
    (void)src;
    (void)sw;
    (void)sh;
    (void)dst;
    (void)dw;
    (void)dh;
    (void)dstStridePixels;
}

void WiiUWindow::present(const uint32_t* pixels, int srcWidth, int srcHeight) {
    (void)pixels;
    (void)srcWidth;
    (void)srcHeight;
}

#endif
