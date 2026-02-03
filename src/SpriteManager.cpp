#include "SpriteManager.h"
#include "game_bin.h"
#include <cstddef>

SpriteManager::SpriteManager() {
    loadSpriteTable();
}

void SpriteManager::loadSpriteTable() {
    mSpriteTable.clear();

    extern unsigned char game_bin[];
    extern unsigned int game_bin_len;

    std::size_t offset = 0;
    std::size_t totalPixels = game_bin_len / sizeof(uint32_t);

    while (offset + 32*32 <= totalPixels) {
        SpriteMeta meta;
        meta.offset = offset;
        meta.width = 32;
        meta.height = 32;
        meta.frames = 1;
        meta.x = 0;
        meta.y = 0;
        meta.currentFrame = 0;
        meta.paletteOffset = 0;

        mSpriteTable.push_back(meta);
        offset += 32*32;
    }
}
