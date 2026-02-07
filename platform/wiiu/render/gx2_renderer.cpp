#include "gx2_renderer.h"

#if defined(PLATFORM_WIIU)
#include "wiiu/WiiUGfx.h"
#include "rmxmedia/framework/GX2Renderer.h"
#include "rmxbase/tools/Logging.h"
#include <cstring>

namespace {
    bool sInitialized = false;
    int sWidth = 0;
    int sHeight = 0;
}

namespace render {

bool initialize_gx2_renderer(int width, int height)
{
    if (sInitialized) return true;
    if (!rmx::WiiUGfx::isGX2Active())
    {
        RMX_LOG_WARNING("gx2_renderer: GX2 not active, falling back to OSScreen");
        return false;
    }
    sWidth = width;
    sHeight = height;
    auto& r = rmx::GX2Renderer::instance();
    if (!r.initialize(width, height))
    {
        RMX_LOG_ERROR("gx2_renderer: GX2Renderer::initialize failed");
        return false;
    }
    sInitialized = true;
    RMX_LOG_INFO("gx2_renderer: initialized " << width << "x" << height);
    return true;
}

void shutdown_gx2_renderer()
{
    if (!sInitialized) return;
    rmx::GX2Renderer::instance().shutdown();
    sInitialized = false;
}

void gx2_begin_frame()
{
    if (!sInitialized) return;
    rmx::GX2Renderer::instance().beginFrame();
}

void gx2_end_frame()
{
    if (!sInitialized) return;
    rmx::GX2Renderer::instance().endFrame();
}

void gx2_present_framebuffer(const uint32_t* pixels, int width, int height)
{
    if (!pixels) return;
    rmx::WiiUGfx::present(pixels, width, height);
}

void gx2_fill_rect(int x, int y, int w, int h, uint32_t rgba)
{
    if (!sInitialized) return;
    rmx::GX2Renderer::instance().drawTexturedQuad(x, y, w, h, 0, rgba);
}

bool gx2_is_active()
{
    return sInitialized && rmx::WiiUGfx::isGX2Active();
}

} // namespace render

#else

namespace render {

bool initialize_gx2_renderer(int, int) { return false; }
void shutdown_gx2_renderer() {}
void gx2_begin_frame() {}
void gx2_end_frame() {}
void gx2_present_framebuffer(const uint32_t*, int, int) {}
void gx2_fill_rect(int, int, int, int, uint32_t) {}
bool gx2_is_active() { return false; }

} // namespace render

#endif
