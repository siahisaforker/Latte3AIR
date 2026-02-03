#pragma once
#include <cstdint>
#include <vector>
#include "Geometry.h"

class Renderer
{
public:
    Renderer() {}
    ~Renderer();

    void initialize();
    void reset();
    void setGameResolution(int width, int height);
    void clearGameScreen();
    void renderGameScreen(const std::vector<Geometry*>& geometries);
    void renderDebugDraw(int debugDrawMode);
    const uint32_t* getFrameBuffer() const { return mFrameBuffer; }
    int getWidth() const { return mWidth; }
    int getHeight() const { return mHeight; }

private:
    uint32_t* mFrameBuffer = nullptr;
    int mWidth = 1280;
    int mHeight = 720;
};
