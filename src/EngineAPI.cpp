#include "EngineAPI.h"
#include <thread>
#include <chrono>

#if defined(PLATFORM_WIIU) || defined(__WIIU__)
#include "platform/wiiu/render/renderer_instance.h"
#include "platform/wiiu/render/renderer.h"
#include "platform/wiiu/input/input_state.h"
#endif

EngineAPI::EngineAPI() {}

EngineAPI::~EngineAPI() {
#if !defined(PLATFORM_WIIU) && !defined(__WIIU__)
    mWindow.shutdown();
#endif
}

bool EngineAPI::initialize() {
    mRenderer.initialize();
    mRenderer.setGameResolution(640, 480);
    mRenderer.clearGameScreen();
#if !defined(PLATFORM_WIIU) && !defined(__WIIU__)
    mWindow.initialize(mRenderer.getWidth(), mRenderer.getHeight());
#endif
    return true;
}

void EngineAPI::runFrame() {
#if defined(PLATFORM_WIIU) || defined(__WIIU__)
    // Poll Wii U input and feed to engine
    InputState inputState;
    pollWiiUInput(inputState);
    uint16_t inputFlags = inputStateToOxygenFlags(inputState);
    
    // Inject input into engine (this would need to be implemented in Oxygen)
    // mInputManager.injectInput(inputFlags);
#endif

    mRenderer.clearGameScreen();
    mSpriteManager.updateEngineFrame();
    auto sprites = mSpriteManager.getFrameSprites();
    mRenderer.renderGameScreen(std::vector<Geometry*>(sprites.begin(), sprites.end()));

    for (auto* s : sprites) delete s;

#if defined(PLATFORM_WIIU) || defined(__WIIU__)
    wiiu::Renderer* platformRenderer = getWiiURenderer();
    if (platformRenderer) {
        const int width = mRenderer.getWidth();
        const int height = mRenderer.getHeight();
        const int pitch = width * static_cast<int>(sizeof(uint32_t));
        platformRenderer->beginFrame();
        platformRenderer->uploadFrameBuffer(
            mRenderer.getFrameBuffer(),
            width,
            height,
            pitch
        );
        platformRenderer->present();
    }
#else
    mWindow.present(mRenderer.getFrameBuffer(), mRenderer.getWidth(), mRenderer.getHeight());
#endif
    std::this_thread::sleep_for(std::chrono::milliseconds(16)); // ~60 FPS
}
