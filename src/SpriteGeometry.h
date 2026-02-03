#pragma once
#include "Geometry.h"
#include "game_bin.h" // include your converted ROM header
#include <cstddef>    // for std::size_t

class SpriteGeometry : public Geometry {
public:
    SpriteGeometry(int offset, int w, int h)
        : Geometry(Type::SPRITE), x(0), y(0),
          width(w), height(h),
          currentFrame(0), totalFrames(1),
          paletteOffset(0),
          spriteOffset(offset) {}

    const uint32_t* getPixelData() const {
        extern unsigned char game_bin[];  // ensure your bin2c output matches this name
        std::size_t frameOffset = spriteOffset + currentFrame * width * height;
        return reinterpret_cast<const uint32_t*>(&game_bin[frameOffset]);
    }

    void setPosition(int px, int py) { x = px; y = py; }
    void setAnimation(int frame, int total) { currentFrame = frame; totalFrames = total; }
    void setPaletteOffset(int offset) { paletteOffset = offset; }

    int x, y;
    int width, height;

private:
    int currentFrame;
    int totalFrames;
    int paletteOffset;
    int spriteOffset;
};
