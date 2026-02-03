/**
 * Wii U stub SpriteManager implementation
 * Provides only basic stub functions to avoid crashes
 */

#if defined(PLATFORM_WIIU) || defined(__WIIU__)

#include "SpriteManager.h"
#include <vector>

SpriteManager::SpriteManager() {}

void SpriteManager::updateEngineFrame() {
    // No-op for stub
}

std::vector<SpriteGeometry*> SpriteManager::getFrameSprites() {
    // Return empty vector for stub
    return std::vector<SpriteGeometry*>();
}

void SpriteManager::loadSpriteTable() {
    // No-op for stub
}

#else

// Keep original implementation for other platforms
SpriteManager::SpriteManager() {}

void SpriteManager::updateEngineFrame() {
    // Original implementation for other platforms
}

std::vector<SpriteGeometry*> SpriteManager::getFrameSprites() {
    // Original implementation for other platforms
    return std::vector<SpriteGeometry*>();
}

void SpriteManager::loadSpriteTable() {
    // Original implementation for other platforms
}

#endif
