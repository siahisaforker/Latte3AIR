/**
 * Wii U static test - avoids dynamic loading
 * Tests if the crash is caused by OSDynLoad functions
 */

#if defined(PLATFORM_WIIU) || defined(__WIIU__)

// Only basic includes, avoid anything that might trigger dynamic loading
#include <coreinit/screen.h>
#include <coreinit/memory.h>
#include <malloc.h>

// Standard WUT entry point
extern "C" int main(int argc, char** argv) {
    (void)argc;
    (void)argv;
    
    // Only use OSScreen - avoid anything that might trigger dynamic loading
    OSScreenInit();
    
    // Get buffer sizes
    uint32_t tvBufferSize = OSScreenGetBufferSizeEx(SCREEN_TV);
    uint32_t drcBufferSize = OSScreenGetBufferSizeEx(SCREEN_DRC);
    
    if (tvBufferSize == 0 || drcBufferSize == 0) {
        return -1;
    }
    
    // Allocate buffers
    void* tvBuffer = memalign(0x100, (size_t)tvBufferSize);
    void* drcBuffer = memalign(0x100, (size_t)drcBufferSize);
    
    if (!tvBuffer || !drcBuffer) {
        if (tvBuffer) free(tvBuffer);
        if (drcBuffer) free(drcBuffer);
        return -1;
    }
    
    // Set buffers
    OSScreenSetBufferEx(SCREEN_TV, tvBuffer);
    OSScreenSetBufferEx(SCREEN_DRC, drcBuffer);
    
    // Show success - blue screen
    OSScreenClearBufferEx(SCREEN_TV, 0x000000FF); // Blue background
    OSScreenClearBufferEx(SCREEN_DRC, 0x000000FF); // Blue background
    OSScreenFlipBuffersEx(SCREEN_TV);
    OSScreenFlipBuffersEx(SCREEN_DRC);
    
    // Wait a bit
    for (volatile int i = 0; i < 5000000; ++i) {
        // Simple delay loop
    }
    
    // Cleanup
    if (tvBuffer) free(tvBuffer);
    if (drcBuffer) free(drcBuffer);
    
    return 0;
}

#endif
