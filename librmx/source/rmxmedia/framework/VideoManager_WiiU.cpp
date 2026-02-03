/*
 * Wii U specific VideoManager implementation (skeleton)
 * - Uses WUT/GX2 for rendering. This file provides a minimal, buildable
 *   skeleton and TODOs for replacing with a full GX2 renderer.
 *
 * NOTE: The exact WUT header paths and function names may differ depending
 * on your devkit/WUT version. Adjust the includes and initialization calls
 * accordingly.
 */

#include "rmxmedia.h"
#include "rmxmedia/framework/VideoManager.h"
#include "rmxmedia/framework/GX2Renderer.h"

#if defined(PLATFORM_WIIU)

#if defined(__has_include)
# if __has_include(<gx2.h>)
#  include <gx2.h>
# else
#  include "wiiu_shim_gx2.h"
# endif
#else
# include "wiiu_shim_gx2.h"
#endif
#include <memory>
#include <chrono>
#include <thread>

// Frame pacing duration (default 60 FPS)
static std::chrono::duration<double> s_frameDuration = std::chrono::duration<double>(1.0/60.0);

namespace rmx
{

VideoManager::VideoManager()
{
}

VideoManager::~VideoManager()
{
    // Clean up GX2 / WHB resources here
    if (mInitialized)
    {
        // Terminate WHB/GX2
        WHBGfxTerm();
        WHBProcShutdown();
    }
}

bool VideoManager::setVideoMode(const VideoConfig& videoconfig)
{
    // On Wii U we don't create an SDL window. Store config and use GX2 surfaces.
    mVideoConfig = videoconfig;
    // TODO: resize or reallocate render targets if resolution changed
    return true;
}

bool VideoManager::initialize(const VideoConfig& videoconfig)
{
    // Initialize platform graphics (WHB + GX2)
    // The WHB and GX2 initialization functions below are typical for WUT,
    // but please consult your WUT/devkit documentation and headers.

    if (!FTX::System->initialize())
        return false;

    // Initialize the WHB window/graphics subsystem
    WHBProcInitialize();
    if (WHBGfxInit() != 0)
    {
        RMX_ERROR("WHBGfxInit failed", );
        return false;
    }

    // Initialize low-level GX2 state
    GX2Init();

    // Store config
    mVideoConfig = videoconfig;

    // Default to 60 Hz behaviour; ensure our frame pacing matches 60fps
    s_frameDuration = std::chrono::duration<double>(1.0 / 60.0);

    // Allocate/render target setup: WHB provides default framebuffers; for more
    // advanced usage create GX2 textures and attach them here. For now rely on
    // WHB's default swapchain and render into the current framebuffer.

    mInitialized = true;
    // Initialize our renderer wrapper to match requested resolution
    GX2Renderer::instance().initialize(mVideoConfig.mWindowRect.width, mVideoConfig.mWindowRect.height);
    return true;
}

void VideoManager::setInitialized(const VideoConfig& videoconfig, SDL_Window* window)
{
    // Not applicable on Wii U; just set state
    mVideoConfig = videoconfig;
    mInitialized = true;
}

void VideoManager::reshape(int width, int height)
{
    if (mVideoConfig.mWindowRect.width == width && mVideoConfig.mWindowRect.height == height)
        return;

    mVideoConfig.mWindowRect.width = width;
    mVideoConfig.mWindowRect.height = height;
    mReshaped = true;

    // TODO: reallocate GX2 render targets here
}

void VideoManager::beginRendering()
{
    // Bind the default framebuffer and clear if requested.
    // WHB/GX2 typically provide the current render target via WHBGfxGetCurrentFramebuffer
    if (mVideoConfig.mAutoClearScreen)
    {
        GX2SetClearColor(0x00000000);
        // Clear via GX2 wrapper
        GX2Clear(GX2_CLEAR_ALL);
    }

    // Begin frame in renderer wrapper
    GX2Renderer::instance().beginFrame();
}

void VideoManager::endRendering()
{
    // Flush GPU caches and present the framebuffer.
    GX2Flush(); // Ensure commands are flushed

    // Let our renderer finish frame (no-op currently) and present
    GX2Renderer::instance().endFrame();
    WHBGfxSwapBuffers();

    // Simple frame pacing to target 60 FPS when not synchronized by WHB.
    static auto lastFrame = std::chrono::steady_clock::now();
    auto now = std::chrono::steady_clock::now();
    auto elapsed = now - lastFrame;
    if (elapsed < s_frameDuration)
    {
        std::this_thread::sleep_for(s_frameDuration - elapsed);
    }
    lastFrame = std::chrono::steady_clock::now();

    mReshaped = false;
}

void VideoManager::setPixelView()
{
    // Set up an orthographic projection for 2D rendering.
    // Implementation depends on the shader uniforms used by the Painter.
}

void VideoManager::setPerspective2D(double fov, double dnear, double dfar)
{
    // TODO: Implement if required for GX2 pipeline
}

void VideoManager::getScreenBitmap(Bitmap& bitmap)
{
    // Reading back framebuffer on Wii U can be expensive — implement if needed
}

uint64 VideoManager::getNativeWindowHandle() const
{
    // Not applicable on Wii U
    return 0;
}

} // namespace rmx

#endif // PLATFORM_WIIU
