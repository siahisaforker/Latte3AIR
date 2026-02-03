#pragma once
#include "SpriteGeometry.h"
#include <vector>

struct SpriteMeta {
    int offset;
    int width;
    int height;
    int frames;
    int x, y;
    int currentFrame;
    int paletteOffset;
};

class SpriteManager {
public:
    SpriteManager();
    void updateEngineFrame(); // updates positions/animations
    std::vector<SpriteGeometry*> getFrameSprites();

private:
    std::vector<SpriteMeta> mSpriteTable;
    void loadSpriteTable();
};
