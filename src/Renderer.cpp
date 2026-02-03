#include "Renderer.h"
#include "SpriteGeometry.h"
#include <cstring>
#include <cstdlib> // malloc/free

#if defined(PLATFORM_WIIU) || defined(__WIIU__)
#include "platform/wiiu/endianness.h"
#include "platform/wiiu/crash_prevention.h"
#include <malloc.h>
#endif

void Renderer::initialize() {
#if defined(PLATFORM_WIIU) || defined(__WIIU__)
    // Use aligned memory for Wii U GPU buffers
    mFrameBuffer = (uint32_t*)memalign(64, mWidth * mHeight * sizeof(uint32_t));
#else
    mFrameBuffer = (uint32_t*)std::malloc(mWidth * mHeight * sizeof(uint32_t));
#endif
    clearGameScreen();
}

Renderer::~Renderer() {
    if (mFrameBuffer) {
        std::free(mFrameBuffer);
        mFrameBuffer = nullptr;
    }
}

void Renderer::reset() {
    clearGameScreen();
}

void Renderer::setGameResolution(int width, int height) {
    if (mFrameBuffer) std::free(mFrameBuffer);
    mWidth = width;
    mHeight = height;
    
#if defined(PLATFORM_WIIU) || defined(__WIIU__)
    // Use aligned memory for Wii U GPU buffers
    mFrameBuffer = (uint32_t*)memalign(64, mWidth * mHeight * sizeof(uint32_t));
#else
    mFrameBuffer = (uint32_t*)std::malloc(mWidth * mHeight * sizeof(uint32_t));
#endif
    clearGameScreen();
}

void Renderer::clearGameScreen() {
    if (mFrameBuffer)
        std::memset(mFrameBuffer, 0, mWidth * mHeight * sizeof(uint32_t));
}

void Renderer::renderGameScreen(const std::vector<Geometry*>& geometries) {
    if (!mFrameBuffer) return;

    for (auto* geo : geometries) {
        if (!geo) continue;
        if (geo->getType() != Geometry::Type::SPRITE) continue;

        const SpriteGeometry* sprite = static_cast<const SpriteGeometry*>(geo);
        int sx = sprite->x;
        int sy = sprite->y;
        int w = sprite->width;
        int h = sprite->height;
        const uint32_t* srcPixels = sprite->getPixelData();

        for (int y = 0; y < h; ++y) {
            int py = sy + y;
            if (py < 0 || py >= mHeight) continue;
            for (int x = 0; x < w; ++x) {
                int px = sx + x;
                if (px < 0 || px >= mWidth) continue;

                uint32_t color = srcPixels[y * w + x];
                if (color != 0)
                    mFrameBuffer[py * mWidth + px] = color;
            }
        }
    }
}

void Renderer::renderDebugDraw(int debugDrawMode) {
    // optional debug rendering, leave empty for now
}
