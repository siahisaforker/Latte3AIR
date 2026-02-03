#pragma once

#include <cstdint>

namespace rmx
{
	class WiiUGfx
	{
	public:
		static bool initialize(int srcWidth, int srcHeight);
		static void shutdown();
		static void present(const uint32_t* pixels, int srcWidth, int srcHeight);
		static bool isGX2Active();
	};
}
