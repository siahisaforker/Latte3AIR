#pragma once
#include <cstdint>

namespace render {

/// Initialize the OSScreen (simple framebuffer) renderer — used as fallback
/// when GX2 is unavailable.
bool initialize_osscreen_renderer(int width, int height);

/// Shut down OSScreen renderer.
void shutdown_osscreen_renderer();

/// Present a software-rendered framebuffer to TV + DRC via OSScreen.
void osscreen_present_framebuffer(const uint32_t* pixels, int width, int height);

/// Print a single text string at character column/row — useful for debug.
void osscreen_put_text(int col, int row, const char* text);

/// Clear both screens to a solid colour.
void osscreen_clear(uint32_t rgba);

/// Flip OSScreen buffers (double-buffering).
void osscreen_flip();

} // namespace render
