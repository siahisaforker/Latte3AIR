#include "osscreen_renderer.h"

#if defined(PLATFORM_WIIU)
#include "wiiu/WiiUGfx.h"
#include "rmxbase/tools/Logging.h"
#include <coreinit/screen.h>
#include <coreinit/cache.h>
#include <cstring>

namespace {
    bool sInitialized = false;
    int sWidth = 0;
    int sHeight = 0;
}

namespace render {

bool initialize_osscreen_renderer(int width, int height)
{
    if (sInitialized) return true;
    sWidth = width;
    sHeight = height;
    // OSScreen init is handled by WiiUGfx::initialize()
    if (!rmx::WiiUGfx::isGX2Active())
    {
        sInitialized = true;
        RMX_LOG_INFO("osscreen_renderer: initialized " << width << "x" << height);
        return true;
    }
    // GX2 mode — don't use OSScreen
    return false;
}

void shutdown_osscreen_renderer()
{
    if (!sInitialized) return;
    sInitialized = false;
}

void osscreen_present_framebuffer(const uint32_t* pixels, int width, int height)
{
    if (!sInitialized || !pixels) return;
    rmx::WiiUGfx::present(pixels, width, height);
}

void osscreen_put_text(int col, int row, const char* text)
{
    if (!sInitialized || !text) return;
    OSScreenPutFontEx(SCREEN_TV, static_cast<uint32_t>(col), static_cast<uint32_t>(row), text);
    OSScreenPutFontEx(SCREEN_DRC, static_cast<uint32_t>(col), static_cast<uint32_t>(row), text);
}

void osscreen_clear(uint32_t rgba)
{
    if (!sInitialized) return;
    OSScreenClearBufferEx(SCREEN_TV, rgba);
    OSScreenClearBufferEx(SCREEN_DRC, rgba);
}

void osscreen_flip()
{
    if (!sInitialized) return;
    OSScreenFlipBuffersEx(SCREEN_TV);
    OSScreenFlipBuffersEx(SCREEN_DRC);
}

} // namespace render

#else

namespace render {

bool initialize_osscreen_renderer(int, int) { return false; }
void shutdown_osscreen_renderer() {}
void osscreen_present_framebuffer(const uint32_t*, int, int) {}
void osscreen_put_text(int, int, const char*) {}
void osscreen_clear(uint32_t) {}
void osscreen_flip() {}

} // namespace render

#endif
