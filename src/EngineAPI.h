#pragma once
#include "Renderer.h"
#include "SpriteManager.h"
#include "WiiUWindow.h"

class EngineAPI {
public:
    EngineAPI();
    ~EngineAPI();

    void initialize();
    void runFrame();

private:
    Renderer mRenderer;
    SpriteManager mSpriteManager;
    WiiUWindow mWindow;
};
