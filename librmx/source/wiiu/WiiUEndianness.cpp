#include "WiiUEndianness.h"
#include <malloc.h>

namespace rmx {

// Static member definitions for memory pool
uint8_t* WiiUEndianness::sMemoryPool = nullptr;
size_t WiiUEndianness::sPoolOffset = 0;
bool WiiUEndianness::sPoolInitialized = false;

void* WiiUEndianness::alignedAlloc(size_t size, size_t alignment)
{
    // Use memalign for aligned allocation on Wii U
    return memalign(alignment, size);
}

void WiiUEndianness::alignedFree(void* ptr)
{
    free(ptr);
}

void* WiiUEndianness::poolAlloc(size_t size)
{
    // Align size to 8-byte boundary
    size = alignUp(size, 8);
    
    if (!sPoolInitialized)
    {
        sMemoryPool = static_cast<uint8_t*>(alignedAlloc(kPoolSize, 0x40));
        if (sMemoryPool)
        {
            sPoolOffset = 0;
            sPoolInitialized = true;
        }
        else
        {
            return nullptr;
        }
    }
    
    if (sPoolOffset + size > kPoolSize)
    {
        // Pool exhausted, fall back to regular allocation
        return alignedAlloc(size, 0x40);
    }
    
    void* ptr = sMemoryPool + sPoolOffset;
    sPoolOffset += size;
    return ptr;
}

void WiiUEndianness::poolFree(void* ptr)
{
    // Simple pool implementation - no individual frees
    // Use cleanupPool() to free all pool memory at once
    (void)ptr;
}

void WiiUEndianness::cleanupPool()
{
    if (sMemoryPool)
    {
        alignedFree(sMemoryPool);
        sMemoryPool = nullptr;
    }
    sPoolOffset = 0;
    sPoolInitialized = false;
}

} // namespace rmx
