#include "Renderer.h"
#include "SpriteGeometry.h"
#include <cstring>
#include <cstdlib> // malloc/free

void Renderer::initialize() {
    mFrameBuffer = (uint32_t*)std::malloc(mWidth * mHeight * sizeof(uint32_t));
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
    mFrameBuffer = (uint32_t*)std::malloc(mWidth * mHeight * sizeof(uint32_t));
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
