/*
*	rmx Library
*	Copyright (C) 2008-2025 by Eukaryot
*
*	Published under the GNU GPLv3 open source software license, see license.txt
*	or https://www.gnu.org/licenses/gpl-3.0.en.html
*/

#pragma once


#include "rmxbase/base/Types.h"

#if defined(PLATFORM_VITA)
	#include <psp2/kernel/clib.h>
#endif


namespace rmx
{
#if RMX_IS_BIG_ENDIAN
	// On big-endian hosts, "swapped" read from big-endian emulated memory is just a plain read
	template<typename T>
	T readMemoryUnaligned(const void* pointer)
	{
		T val;
		__builtin_memcpy(&val, pointer, sizeof(T));
		return val;
	}

	template<typename T>
	T readMemoryUnalignedSwapped(const void* pointer)
	{
		T val;
		__builtin_memcpy(&val, pointer, sizeof(T));
		return val;  // No swap needed — data is already in native byte order
	}

#elif !defined(__vita__)
	template<typename T>
	T readMemoryUnaligned(const void* pointer) { return *(T*)pointer; }

	template<typename T>
	T readMemoryUnalignedSwapped(const void* pointer) { return swapBytes<T>(*(T*)pointer); }
#else
	template<typename T>
	T readMemoryUnaligned(const void* pointer)
	{
		T val;
		sceClibMemcpy(&val, pointer, sizeof(T));
		return val;
	}

	template<typename T>
	T readMemoryUnalignedSwapped(const void* pointer)
	{
		T val;
		sceClibMemcpy(&val, pointer, sizeof(T));
		return swapBytes<T>(val);
	}
#endif

#if defined(__arm__) && !defined(__vita__)
	template<> uint16 readMemoryUnaligned(const void* pointer);
	template<> uint32 readMemoryUnaligned(const void* pointer);
	template<> uint64 readMemoryUnaligned(const void* pointer);

	template<> uint16 readMemoryUnalignedSwapped(const void* pointer);
	template<> uint32 readMemoryUnalignedSwapped(const void* pointer);
	template<> uint64 readMemoryUnalignedSwapped(const void* pointer);
#endif
}
