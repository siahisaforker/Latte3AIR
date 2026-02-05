// WHB/GX2 shim implementations for Create/Update/Destroy/Bind/Draw helpers
#include "wiiu_shim_gx2.h"
#include "GX2Renderer.h"

#include <unordered_map>
#include <vector>
#include <mutex>
#include <cstdint>
#include <cstring>
#include "oxygen/helper/Logging.h"

# if WIIU_HAS_WHB_HEADERS
# include <coreinit/memdefaultheap.h>
# include <gx2/texture.h>
# include <gx2/surface.h>
# include <gx2/enum.h>
# include <gx2/state.h>
#endif

namespace {
#if WIIU_HAS_WHB_HEADERS
    struct ShimTex { int id; GX2Texture* texture; void* imageMem; size_t imageSize; };
#else
    struct ShimTex { int id; int w; int h; std::vector<uint32_t> pixels; };
#endif
    std::unordered_map<int, ShimTex> s_textures;
    int s_nextId = 1;
    int s_bound = 0;
    std::mutex s_mutex;
}

extern "C" {

int WHBGfxCreateTexture(int w, int h, int /*fmt*/, const void* data)
{
    if (w <= 0 || h <= 0) return -1;
    std::lock_guard<std::mutex> lock(s_mutex);
    int id = s_nextId++;
    RMX_LOG_INFO("WHBGfxCreateTexture: requested " << w << "x" << h << " id=" << id);
#if WIIU_HAS_WHB_HEADERS
    ShimTex t; t.id = id; t.texture = nullptr; t.imageMem = nullptr; t.imageSize = 0;

    // Allocate GX2Texture structure
    GX2Texture* gxTex = static_cast<GX2Texture*>(MEMAllocFromDefaultHeapEx(sizeof(GX2Texture), 0x40));
    if (!gxTex)
    {
        RMX_LOG_ERROR("WHBGfxCreateTexture: failed to allocate GX2Texture (w=" << w << ", h=" << h << ")");
        return -1;
    }
    std::memset(gxTex, 0, sizeof(GX2Texture));

    // Configure surface
    GX2Surface* surf = &gxTex->surface;
    surf->dim = GX2_SURFACE_DIM_TEXTURE_2D;
    surf->width = static_cast<uint32_t>(w);
    surf->height = static_cast<uint32_t>(h);
    surf->format = GX2_SURFACE_FORMAT_UNORM_R8_G8_B8_A8;

    // Calculate required image size/pitch/alignment (fills surface fields)
    GX2CalcSurfaceSizeAndAlignment(surf);

    size_t imageSize = static_cast<size_t>(surf->imageSize);
    size_t alignment = static_cast<size_t>(surf->alignment ? surf->alignment : 0x40);

    void* img = MEMAllocFromDefaultHeapEx(imageSize, static_cast<uint32_t>(alignment));
    if (!img)
    {
        RMX_LOG_ERROR("WHBGfxCreateTexture: failed to allocate image memory size=" << imageSize << " alignment=" << alignment);
        MEMFreeToDefaultHeap(gxTex);
        return -1;
    }

    surf->image = img;

    // If initial data provided, perform a simple linear copy assuming RGBA8 linear layout.
    if (data)
    {
        const uint8_t* src = static_cast<const uint8_t*>(data);
        // Copy row by row into the texture image (may not be optimal for tiled surfaces)
        const size_t rowBytes = static_cast<size_t>(w) * 4u;
        const size_t pitch = static_cast<size_t>(surf->pitch ? surf->pitch : rowBytes);
        for (uint32_t row = 0; row < surf->height; ++row)
        {
            void* dstRow = static_cast<uint8_t*>(img) + static_cast<size_t>(row) * pitch;
            std::memcpy(dstRow, src + static_cast<size_t>(row) * rowBytes, rowBytes);
        }
    }

    // Initialize texture registers so GX2 can sample it
    GX2InitTextureRegs(gxTex);
    RMX_LOG_INFO("WHBGfxCreateTexture: created id=" << id << " imageSize=" << imageSize << " pitch=" << surf->pitch);
    t.texture = gxTex;
    t.imageMem = img;
    t.imageSize = imageSize;
    s_textures[id] = std::move(t);
    return id;
#else
    ShimTex t; t.id = id; t.w = w; t.h = h; t.pixels.resize(static_cast<size_t>(w) * static_cast<size_t>(h));
    if (data)
    {
        const uint8_t* src = static_cast<const uint8_t*>(data);
        size_t idx = 0;
        for (int y = 0; y < h; ++y)
        {
            for (int x = 0; x < w; ++x)
            {
                uint8_t r = src[idx*4 + 0];
                uint8_t g = src[idx*4 + 1];
                uint8_t b = src[idx*4 + 2];
                uint8_t a = src[idx*4 + 3];
                t.pixels[y * w + x] = (static_cast<uint32_t>(a) << 24) | (static_cast<uint32_t>(b) << 16) | (static_cast<uint32_t>(g) << 8) | (static_cast<uint32_t>(r));
                ++idx;
            }
        }
    }
    s_textures[id] = std::move(t);
    return id;
#endif
}

void WHBGfxUpdateTexture(int tex, int x, int y, int w, int h, const void* data)
{
    std::lock_guard<std::mutex> lock(s_mutex);
    auto it = s_textures.find(tex);
    if (it == s_textures.end()) return;
#if WIIU_HAS_WHB_HEADERS
    ShimTex &t = it->second;
    if (!t.texture)
    {
        RMX_LOG_WARNING("WHBGfxUpdateTexture: texture handle " << tex << " has no GX2 texture");
        return;
    }
    if (!t.imageMem)
    {
        RMX_LOG_WARNING("WHBGfxUpdateTexture: texture handle " << tex << " has no image memory");
        return;
    }
    if (!data)
    {
        RMX_LOG_WARNING("WHBGfxUpdateTexture: no data provided for tex=" << tex);
        return;
    }
    GX2Surface* surf = &t.texture->surface;
    const uint8_t* src = static_cast<const uint8_t*>(data);
    const size_t rowBytes = static_cast<size_t>(w) * 4u;
    const size_t pitch = static_cast<size_t>(surf->pitch ? surf->pitch : rowBytes);
    for (int row = 0; row < h; ++row)
    {
        int dstY = y + row; if (dstY < 0 || dstY >= static_cast<int>(surf->height)) continue;
        void* dstRow = static_cast<uint8_t*>(t.imageMem) + static_cast<size_t>(dstY) * pitch + static_cast<size_t>(x) * 4u;
        std::memcpy(dstRow, src + static_cast<size_t>(row) * rowBytes, rowBytes);
    }
    // Inform GPU of changes - flush CPU cache so GPU sees the upload
    GX2Flush();
    RMX_LOG_TRACE("WHBGfxUpdateTexture: updated tex=" << tex << " rect=(" << x << "," << y << "," << w << "," << h << ")");
#else
    ShimTex &t = it->second;
    if (!data) { RMX_LOG_WARNING("WHBGfxUpdateTexture: no data provided for cpu texture=" << tex); return; }
    const uint8_t* src = static_cast<const uint8_t*>(data);
    for (int row = 0; row < h; ++row)
    {
        int dstY = y + row; if (dstY < 0 || dstY >= t.h) continue;
        for (int col = 0; col < w; ++col)
        {
            int dstX = x + col; if (dstX < 0 || dstX >= t.w) continue;
            size_t sidx = static_cast<size_t>(row * w + col) * 4;
            uint8_t r = src[sidx+0]; uint8_t g = src[sidx+1]; uint8_t b = src[sidx+2]; uint8_t a = src[sidx+3];
            t.pixels[dstY * t.w + dstX] = (static_cast<uint32_t>(a) << 24) | (static_cast<uint32_t>(b) << 16) | (static_cast<uint32_t>(g) << 8) | (static_cast<uint32_t>(r));
        }
    }
#endif
}

void WHBGfxDestroyTexture(int tex)
{
    std::lock_guard<std::mutex> lock(s_mutex);
    auto it = s_textures.find(tex);
    if (it == s_textures.end()) return;
#if WIIU_HAS_WHB_HEADERS
    ShimTex &t = it->second;
    if (t.imageMem)
    {
        RMX_LOG_INFO("WHBGfxDestroyTexture: freeing imageMem for tex=" << tex);
        MEMFreeToDefaultHeap(t.imageMem);
    }
    if (t.texture)
    {
        RMX_LOG_INFO("WHBGfxDestroyTexture: freeing GX2Texture for tex=" << tex);
        MEMFreeToDefaultHeap(t.texture);
    }
#endif
    s_textures.erase(it);
    if (s_bound == tex) s_bound = 0;
}

void WHBGfxBindTexture(int handle)
{
    std::lock_guard<std::mutex> lock(s_mutex);
    if (handle == 0 || s_textures.find(handle) == s_textures.end())
    {
        s_bound = 0;
        RMX_LOG_TRACE("WHBGfxBindTexture: unbound");
    }
    else
    {
        s_bound = handle;
        RMX_LOG_TRACE("WHBGfxBindTexture: bound tex=" << handle);
    }
}

void WHBGfxUnbindTexture()
{
    std::lock_guard<std::mutex> lock(s_mutex);
    s_bound = 0;
}

void WHBGfxDrawTexturedTriangle(int x0,int y0,float u0,float v0,int x1,int y1,float u1,float v1,int x2,int y2,float u2,float v2)
{
    std::lock_guard<std::mutex> lock(s_mutex);
    if (s_bound == 0) return;
    auto it = s_textures.find(s_bound);
    if (it == s_textures.end()) return;
    // Approximate by drawing the bounding quad of the triangle using GX2Renderer
    int minx = std::min(std::min(x0,x1), x2);
    int miny = std::min(std::min(y0,y1), y2);
    int maxx = std::max(std::max(x0,x1), x2);
    int maxy = std::max(std::max(y0,y1), y2);
    int w = std::max(1, maxx - minx);
    int h = std::max(1, maxy - miny);
    uint32_t color = 0xFFFFFFFFu;
    rmx::GX2Renderer::instance().drawTexturedQuad(minx, miny, w, h, s_bound, color);
}

} // extern C
