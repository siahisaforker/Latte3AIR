#include "rmxbase.h"
#include "wiiu/WiiUGfx.h"

#if defined(PLATFORM_WIIU)

#include <coreinit/screen.h>
#include <coreinit/cache.h>
#include <malloc.h>
#include <cstring>

#include "rmxbase/tools/Logging.h"

#if defined(__has_include)
	#if __has_include(<whb/gfx.h>)
		#include <whb/gfx.h>
		#define WIIU_HAS_WHB_GFX 1
	#else
		#define WIIU_HAS_WHB_GFX 0
	#endif
#else
	#define WIIU_HAS_WHB_GFX 0
#endif

namespace
{
	struct GfxState
	{
		bool mInitialized = false;
		bool mGX2Active = false;

		void* mTVBuffer = nullptr;
		void* mDRCBuffer = nullptr;
		std::size_t mTVSize = 0;
		std::size_t mDRCSize = 0;
		int mTVWidth = 0;
		int mTVHeight = 0;
		int mDRCWidth = 0;
		int mDRCHeight = 0;
		int mTVStridePixels = 0;
		int mDRCStridePixels = 0;
	};

	GfxState gState;

	void blitScale(const uint32_t* src, int sw, int sh, uint32_t* dst, int dw, int dh, int dstStridePixels)
	{
		if (!src || !dst || sw <= 0 || sh <= 0 || dw <= 0 || dh <= 0)
			return;

		// Optimised path: when source and dest are same size, use direct memcpy per row
		if (sw == dw && sh == dh)
		{
			const size_t rowBytes = static_cast<size_t>(dw) * sizeof(uint32_t);
			for (int y = 0; y < dh; ++y)
			{
				std::memcpy(dst + y * dstStridePixels, src + y * sw, rowBytes);
			}
			return;
		}

		// Pre-compute X lookup table to avoid per-pixel division
		std::vector<int> xMap(dw);
		for (int x = 0; x < dw; ++x)
		{
			xMap[x] = (x * sw) / dw;
		}

		for (int y = 0; y < dh; ++y)
		{
			const int sy = (y * sh) / dh;
			const uint32_t* srcRow = src + sy * sw;
			uint32_t* dstRow = dst + y * dstStridePixels;

			// Process 4 pixels at a time when possible
			int x = 0;
			const int end4 = dw - 3;
			for (; x < end4; x += 4)
			{
				dstRow[x + 0] = srcRow[xMap[x + 0]];
				dstRow[x + 1] = srcRow[xMap[x + 1]];
				dstRow[x + 2] = srcRow[xMap[x + 2]];
				dstRow[x + 3] = srcRow[xMap[x + 3]];
			}
			for (; x < dw; ++x)
			{
				dstRow[x] = srcRow[xMap[x]];
			}
		}
	}

	bool initOSScreen()
	{
		OSScreenInit();

		// Older SDKs used OSScreenGetScreenSizeEx; use known default resolutions
		gState.mTVWidth = 1280;
		gState.mTVHeight = 720;
		gState.mDRCWidth = 854;
		gState.mDRCHeight = 480;

		gState.mTVSize = OSScreenGetBufferSizeEx(SCREEN_TV);
		gState.mDRCSize = OSScreenGetBufferSizeEx(SCREEN_DRC);

		gState.mTVBuffer = memalign(0x100, gState.mTVSize);
		gState.mDRCBuffer = memalign(0x100, gState.mDRCSize);

		if (!gState.mTVBuffer || !gState.mDRCBuffer)
		{
			RMX_LOG_ERROR("initOSScreen: memalign failed (tvSize=" << gState.mTVSize << ", drcSize=" << gState.mDRCSize << ")");
			if (gState.mTVBuffer)
			{
				free(gState.mTVBuffer);
				gState.mTVBuffer = nullptr;
			}
			if (gState.mDRCBuffer)
			{
				free(gState.mDRCBuffer);
				gState.mDRCBuffer = nullptr;
			}
			return false;
		}

		RMX_LOG_INFO("initOSScreen: allocated TV/DRC buffers (tvSize=" << gState.mTVSize << ", drcSize=" << gState.mDRCSize << ")");

		OSScreenSetBufferEx(SCREEN_TV, gState.mTVBuffer);
		OSScreenSetBufferEx(SCREEN_DRC, gState.mDRCBuffer);
		OSScreenEnableEx(SCREEN_TV, 1);
		OSScreenEnableEx(SCREEN_DRC, 1);

		if (gState.mTVHeight > 0)
		{
			const int stride = static_cast<int>(gState.mTVSize / (sizeof(uint32_t) * gState.mTVHeight));
			gState.mTVStridePixels = (stride > 0) ? stride : gState.mTVWidth;
		}
		else
		{
			gState.mTVStridePixels = gState.mTVWidth;
		}

		if (gState.mDRCHeight > 0)
		{
			const int stride = static_cast<int>(gState.mDRCSize / (sizeof(uint32_t) * gState.mDRCHeight));
			gState.mDRCStridePixels = (stride > 0) ? stride : gState.mDRCWidth;
		}
		else
		{
			gState.mDRCStridePixels = gState.mDRCWidth;
		}

		OSScreenClearBufferEx(SCREEN_TV, 0x00000000);
		OSScreenClearBufferEx(SCREEN_DRC, 0x00000000);
		OSScreenFlipBuffersEx(SCREEN_TV);
		OSScreenFlipBuffersEx(SCREEN_DRC);
		return true;
	}

	bool initGX2()
	{
		// Software renderer outputs RGBA pixels to a CPU buffer.
		// OSScreen handles blitting this buffer to TV/DRC correctly.
		// GX2/WHBGfx manages its own color buffers that don't interop
		// with our CPU-rendered framebuffer, so skip GX2 entirely.
		return false;
	}
}

namespace rmx
{
	bool WiiUGfx::initialize(int srcWidth, int srcHeight)
	{
		(void)srcWidth;
		(void)srcHeight;

		if (gState.mInitialized)
			return true;

		if (initGX2())
		{
			gState.mGX2Active = true;
			gState.mInitialized = true;
			return true;
		}

		if (initOSScreen())
		{
			gState.mGX2Active = false;
			gState.mInitialized = true;
			return true;
		}

		return false;
	}

	void WiiUGfx::shutdown()
	{
		if (!gState.mInitialized)
			return;

#if WIIU_HAS_WHB_GFX
		if (gState.mGX2Active)
		{
			WHBGfxShutdown();
			gState.mTVBuffer = nullptr;
			gState.mDRCBuffer = nullptr;
		}
#endif

		if (!gState.mGX2Active)
		{
			if (gState.mTVBuffer)
			{
				free(gState.mTVBuffer);
				gState.mTVBuffer = nullptr;
			}
			if (gState.mDRCBuffer)
			{
				free(gState.mDRCBuffer);
				gState.mDRCBuffer = nullptr;
			}
		}

		gState = GfxState();
	}

	void WiiUGfx::present(const uint32_t* pixels, int srcWidth, int srcHeight)
	{
		if (!gState.mInitialized || !pixels)
			return;

		blitScale(pixels, srcWidth, srcHeight,
			reinterpret_cast<uint32_t*>(gState.mTVBuffer), gState.mTVWidth, gState.mTVHeight, gState.mTVStridePixels);
		blitScale(pixels, srcWidth, srcHeight,
			reinterpret_cast<uint32_t*>(gState.mDRCBuffer), gState.mDRCWidth, gState.mDRCHeight, gState.mDRCStridePixels);

		DCFlushRange(gState.mTVBuffer, gState.mTVSize);
		DCFlushRange(gState.mDRCBuffer, gState.mDRCSize);

		OSScreenFlipBuffersEx(SCREEN_TV);
		OSScreenFlipBuffersEx(SCREEN_DRC);
	}

	bool WiiUGfx::isGX2Active()
	{
		return gState.mGX2Active;
	}
}

#else

namespace rmx
{
	bool WiiUGfx::initialize(int, int) { return false; }
	void WiiUGfx::shutdown() {}
	void WiiUGfx::present(const uint32_t*, int, int) {}
	bool WiiUGfx::isGX2Active() { return false; }
}

#endif
