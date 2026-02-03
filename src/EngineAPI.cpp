#include "EngineAPI.h"

#if defined(PLATFORM_WIIU) || defined(__WIIU__)
#include "platform/wiiu/render/renderer_instance.h"
#include "platform/wiiu/render/renderer.h"
#include "platform/wiiu/input/input_state.h"
#include "platform/wiiu/threading.h"
#else
#include <thread>
#include <chrono>
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
        platformRenderer->present();
        wiiu::sleep(16); // ~60 FPS
    }
#else
    mWindow.present(mRenderer.getFrameBuffer(), mRenderer.getWidth(), mRenderer.getHeight());
    std::this_thread::sleep_for(std::chrono::milliseconds(16)); // ~60 FPS
#endif
}
