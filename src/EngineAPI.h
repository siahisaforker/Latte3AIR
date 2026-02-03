#pragma once
#include "Renderer.h"
#include "SpriteManager.h"

#if !defined(PLATFORM_WIIU) && !defined(__WIIU__)
#include "WiiUWindow.h"
#endif

class EngineAPI {
public:
    EngineAPI();
    ~EngineAPI();

    bool initialize();
    void runFrame();

private:
    Renderer mRenderer;
    SpriteManager mSpriteManager;
#if !defined(PLATFORM_WIIU) && !defined(__WIIU__)
    WiiUWindow mWindow;
#endif
};
