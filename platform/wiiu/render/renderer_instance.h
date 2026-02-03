#pragma once

namespace wiiu { class Renderer; }

/**
 * Single renderer instance for the Wii U platform.
 * Set once at startup by the renderer selection logic. No runtime switching.
 */
void setWiiURenderer(wiiu::Renderer* renderer);
wiiu::Renderer* getWiiURenderer();
