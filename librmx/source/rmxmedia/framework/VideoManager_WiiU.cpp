/*
 * Wii U specific VideoManager implementation.
 *
 * The Oxygen engine uses SDL window / surface calls even in software-renderer mode.
 * On Wii U we provide an SDL shim (librmx/source/wiiu/SDL_shim.*) that maps:
 * - SDL_CreateWindow / SDL_GetWindowSurface / SDL_UpdateWindowSurface
 * to a GX2 (WHB) presentation path with an OSScreen fallback (librmx/source/wiiu/WiiUGfx.cpp).
 *
 * Therefore, VideoManager on Wii U should behave like the default SDL implementation,
 * but without relying on OpenGL and without direct WHB/GX2 calls.
 */

#include "rmxmedia.h"
#include "rmxmedia/framework/VideoManager.h"

#if defined(PLATFORM_WIIU)

namespace rmx
{

VideoManager::VideoManager()
{
}

VideoManager::~VideoManager()
{
    // Window lifetime is managed by the application (EngineMain::destroyWindow)
    // which calls SDL_DestroyWindow. Do not duplicate that here.
}

bool VideoManager::setVideoMode(const VideoConfig& videoconfig)
{
	// On Wii U we still use the SDL shim window, so treat this similarly to the default implementation.
	// Creating a new window here is only needed if someone uses FTX::Video->initialize directly.
	mVideoConfig = videoconfig;
	return true;
}

bool VideoManager::initialize(const VideoConfig& videoconfig)
{
	if (!FTX::System->initialize())
		return false;

	// Only change the mode?
	if (mInitialized)
		return setVideoMode(videoconfig);

	// Create a shim SDL window (this will initialize WiiUGfx internally)
	// Note: EngineMain usually creates the window itself and then calls setInitialized.
	mMainWindow = SDL_CreateWindow(*videoconfig.mCaption,
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		videoconfig.mWindowRect.width, videoconfig.mWindowRect.height,
		0);
	if (nullptr == mMainWindow)
		return false;

	mVideoConfig = videoconfig;
	SDL_GetWindowSize(mMainWindow, &mVideoConfig.mWindowRect.width, &mVideoConfig.mWindowRect.height);
	mInitialized = true;
	return true;
}

void VideoManager::setInitialized(const VideoConfig& videoconfig, SDL_Window* window)
{
	// EngineMain creates the window; just store it here.
	mVideoConfig = videoconfig;
	mMainWindow = window;
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
	// Software rendering draws into the SDL surface and presents via SDL_UpdateWindowSurface
	// (see oxygen/drawing/software/SoftwareDrawer.cpp). Nothing to do here.
}

void VideoManager::endRendering()
{
	// Present is handled by Drawer::presentScreen -> SoftwareDrawer::presentScreen.
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
