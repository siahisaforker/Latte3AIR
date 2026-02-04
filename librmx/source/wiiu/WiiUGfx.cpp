#include "rmxbase.h"
#include "wiiu/WiiUGfx.h"

#if defined(PLATFORM_WIIU)

#include <coreinit/screen.h>
#include <coreinit/cache.h>
#include <malloc.h>
#include <cstring>

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

		for (int y = 0; y < dh; ++y)
		{
			const int sy = (y * sh) / dh;
			const uint32_t* srcRow = src + sy * sw;
			uint32_t* dstRow = dst + y * dstStridePixels;
			for (int x = 0; x < dw; ++x)
			{
				const int sx = (x * sw) / dw;
				dstRow[x] = srcRow[sx];
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
#if WIIU_HAS_WHB_GFX
	WHBGfxInit();

	// WHB does not expose the same helpers in this SDK; use OSScreen defaults
	gState.mTVWidth = 1280;
	gState.mTVHeight = 720;
	gState.mDRCWidth = 854;
	gState.mDRCHeight = 480;

	gState.mTVSize = OSScreenGetBufferSizeEx(SCREEN_TV);
	gState.mDRCSize = OSScreenGetBufferSizeEx(SCREEN_DRC);

	gState.mTVBuffer = memalign(0x100, gState.mTVSize);
	gState.mDRCBuffer = memalign(0x100, gState.mDRCSize);

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

		if (!gState.mTVBuffer || !gState.mDRCBuffer)
		{
			WHBGfxShutdown();
			gState.mTVBuffer = nullptr;
			gState.mDRCBuffer = nullptr;
			gState.mTVSize = 0;
			gState.mDRCSize = 0;
			return false;
		}
		return true;
#else
		return false;
#endif
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

#if WIIU_HAS_WHB_GFX
		if (gState.mGX2Active)
		{
			WHBGfxBeginRender();
			WHBGfxFinishRender();
			WHBGfxFinishRenderTV();
			WHBGfxFinishRenderDRC();
			return;
		}
#endif

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
