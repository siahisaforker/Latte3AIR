/**
 * Wii U stub EngineAPI implementation
 * Provides only basic stub functions to avoid crashes
 */

#include "EngineAPI.h"

#if defined(PLATFORM_WIIU) || defined(__WIIU__)

EngineAPI::EngineAPI() {}

EngineAPI::~EngineAPI() {
    // No-op for stub
}

bool EngineAPI::initialize() {
    // Stub implementation - just return true without initializing anything
    return true;
}

void EngineAPI::runFrame() {
    // No-op for stub
}

#else

// Keep original implementation for other platforms
EngineAPI::EngineAPI() {}

EngineAPI::~EngineAPI() {
    mWindow.shutdown();
}

bool EngineAPI::initialize() {
    mRenderer.initialize();
    mRenderer.setGameResolution(640, 480);
    mRenderer.clearGameScreen();
    mWindow.initialize(mRenderer.getWidth(), mRenderer.getHeight());
    return true;
}

void EngineAPI::runFrame() {
    mWindow.pollEvents();
    mRenderer.clearGameScreen();
    mRenderer.present();
}

#endif
