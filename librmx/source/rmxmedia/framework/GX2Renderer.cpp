/* Lightweight implementation of GX2Renderer using WHB/GX2 shim helpers.
   This is intentionally minimal and structured so a real GX2 implementation
   can be swapped in later.
*/
#include "rmxmedia/framework/GX2Renderer.h"
#include "rmxmedia.h"

#if defined(PLATFORM_WIIU)
# include "wiiu_shim_gx2.h"
#endif

namespace rmx
{

GX2Renderer& GX2Renderer::instance()
{
    static GX2Renderer inst;
    return inst;
}

GX2Renderer::GX2Renderer()
{
}

GX2Renderer::~GX2Renderer()
{
    shutdown();
}

bool GX2Renderer::initialize(int width, int height)
{
    if (mInitialized)
        return true;

    // WHB/GX2 init already performed in VideoManager; just store sizes
    mWidth = width;
    mHeight = height;
    mInitialized = true;
    return true;
}

void GX2Renderer::shutdown()
{
    if (!mInitialized) return;
    // Nothing to free in shim
    mInitialized = false;
}

void GX2Renderer::beginFrame()
{
}

void GX2Renderer::endFrame()
{
}


void GX2Renderer::drawTexturedQuad(int x, int y, int w, int h, int texHandle, uint32_t color)
{
    // Platform-specific helper draws (WHB) are used for now. Pass texture
    // handle (0 = no texture / solid color).
#if defined(PLATFORM_WIIU)
    if (texHandle != 0)
        WHBGfxBindTexture(texHandle);
    WHBGfxDrawTexturedQuad(x, y, w, h, color);
    if (texHandle != 0)
        WHBGfxUnbindTexture();
#else
    (void)x; (void)y; (void)w; (void)h; (void)texHandle; (void)color;
#endif
}

void GX2Renderer::enableScissor(bool enable)
{
#if defined(PLATFORM_WIIU)
    if (!enable)
        WHBGfxDisableScissor();
    else
        ; // keep for setScissor
#else
    (void)enable;
#endif
}

void GX2Renderer::setScissor(int x, int y, int w, int h)
{
#if defined(PLATFORM_WIIU)
    WHBGfxEnableScissor(x, y, w, h);
#else
    (void)x; (void)y; (void)w; (void)h;
#endif
}

} // namespace rmx
