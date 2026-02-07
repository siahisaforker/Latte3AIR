#pragma once
#include <cstdint>

namespace render {

/// Initialize the GX2 rendering backend.
/// Call once after WiiUGfx::initialize().
bool initialize_gx2_renderer(int width, int height);

/// Shut down the GX2 rendering backend.
void shutdown_gx2_renderer();

/// Called at the start of each frame.
void gx2_begin_frame();

/// Called at the end of each frame to flip buffers.
void gx2_end_frame();

/// Software-blit a pixel buffer to both TV + DRC via WiiUGfx::present.
void gx2_present_framebuffer(const uint32_t* pixels, int width, int height);

/// Draw a solid-colour rectangle (for debug overlays).
void gx2_fill_rect(int x, int y, int w, int h, uint32_t rgba);

/// Query whether the GX2 fast-path is active.
bool gx2_is_active();

} // namespace render
