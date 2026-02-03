#include "renderer_instance.h"
#include "renderer.h"

static wiiu::Renderer* sWiiURenderer = nullptr;

void setWiiURenderer(wiiu::Renderer* renderer) {
    sWiiURenderer = renderer;
}

wiiu::Renderer* getWiiURenderer() {
    return sWiiURenderer;
}
