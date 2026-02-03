/**
 * Wii U stub GX2 renderer implementation
 * Provides only basic stub functions to avoid crashes
 */

#if defined(PLATFORM_WIIU) || defined(__WIIU__)

#include "gx2_renderer.h"
#include <malloc.h>
#include <cstring>

namespace wiiu {

GX2Renderer::~GX2Renderer() {
    shutdown();
}

bool GX2Renderer::init() {
    // Stub implementation - just return false to indicate GX2 not available
    return false;
}

void GX2Renderer::shutdown() {
    // No-op for stub
}

void GX2Renderer::beginFrame() {
    // No-op for stub
}

void GX2Renderer::uploadFrameBuffer(const void* pixels, int width, int height, int pitch) {
    (void)pixels;
    (void)width;
    (void)height;
    (void)pitch;
    // No-op for stub
}

void GX2Renderer::present() {
    // No-op for stub
}

} // namespace wiiu

#endif
