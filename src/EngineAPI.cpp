#include "EngineAPI.h"
#include <thread>
#include <chrono>

EngineAPI::EngineAPI() {}

EngineAPI::~EngineAPI() {
    mWindow.shutdown();
}

void EngineAPI::initialize() {
    mRenderer.initialize();
    mRenderer.setGameResolution(640, 480);
    mRenderer.clearGameScreen();
    mWindow.initialize(mRenderer.getWidth(), mRenderer.getHeight());
}

void EngineAPI::runFrame() {
    mRenderer.clearGameScreen();
    mSpriteManager.updateEngineFrame();
    auto sprites = mSpriteManager.getFrameSprites();
    mRenderer.renderGameScreen(std::vector<Geometry*>(sprites.begin(), sprites.end()));

    for (auto* s : sprites) delete s;

    mWindow.present(mRenderer.getFrameBuffer(), mRenderer.getWidth(), mRenderer.getHeight());
    std::this_thread::sleep_for(std::chrono::milliseconds(16)); // ~60 FPS
}
