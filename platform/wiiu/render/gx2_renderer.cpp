#include "gx2_renderer.h"

#if defined(PLATFORM_WIIU) || defined(__WIIU__)

#if defined(__has_include)
#if __has_include(<whb/gfx.h>)
#define WIIU_USE_WHB 1
#endif
#endif

#if defined(WIIU_USE_WHB)
#include <whb/gfx.h>
#include <gx2/state.h>
#include <gx2/context.h>
#include <gx2/texture.h>
#include <gx2/draw.h>
#include <gx2/mem.h>
#include <coreinit/memheap.h>
#include <coreinit/memory.h>
#include <malloc.h>
#include <cstring>
#endif

wiiu::GX2Renderer::~GX2Renderer() {
    shutdown();
}

bool wiiu::GX2Renderer::init() {
#if !defined(WIIU_USE_WHB)
    return false;
#else
    if (mInitialized)
        return true;

    if (WHBGfxInit() != 0)
        return false;

    uint32_t attributes[] = {0};
    GX2Init(attributes);

    mTargetWidth = 1280;
    mTargetHeight = 720;
    mTexWidth = mTargetWidth;
    mTexHeight = mTargetHeight;

    // For now, just use a simple handle placeholder
    mTextureHandle = reinterpret_cast<void*>(0x12345678);

    mInitialized = true;
    return true;
#endif
}

void wiiu::GX2Renderer::shutdown() {
#if defined(WIIU_USE_WHB)
    if (!mInitialized)
        return;
    mTextureHandle = nullptr;
    WHBGfxShutdown();
    mInitialized = false;
#endif
}

void wiiu::GX2Renderer::beginFrame() {
#if defined(WIIU_USE_WHB)
    if (!mInitialized) return;
    WHBGfxBeginRender();
#endif
}

void wiiu::GX2Renderer::uploadFrameBuffer(
    const void* pixels,
    int width,
    int height,
    int pitch
) {
#if defined(WIIU_USE_WHB)
    if (!mInitialized || !pixels || !mTextureHandle) return;

    // For now, just clear to a test color
    WHBGfxClearColor(0.2f, 0.3f, 0.8f, 1.0f);
#endif
    (void)width;
    (void)height;
    (void)pitch;
    (void)pixels;
}

void wiiu::GX2Renderer::present() {
#if defined(WIIU_USE_WHB)
    if (!mInitialized) return;
    
    WHBGfxFinishRender();
    // Swap buffers is handled automatically by WHBGfxFinishRender
#endif
}

#else

wiiu::GX2Renderer::~GX2Renderer() {}
bool wiiu::GX2Renderer::init() { return false; }
void wiiu::GX2Renderer::shutdown() {}
void wiiu::GX2Renderer::beginFrame() {}
void wiiu::GX2Renderer::uploadFrameBuffer(const void*, int, int, int) {}
void wiiu::GX2Renderer::present() {}

#endif
