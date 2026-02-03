/**
 * Wii U simple test - absolute minimum to test if anything runs
 */

#if defined(PLATFORM_WIIU) || defined(__WIIU__)

#include <coreinit/screen.h>
#include <coreinit/memory.h>
#include <malloc.h>
#include <cstdio>

// Standard WUT entry point
extern "C" int main(int argc, char** argv) {
    (void)argc;
    (void)argv;
    
    printf("SIMPLE: Starting test\n");
    
    // Try to initialize OSScreen only
    OSScreenInit();
    printf("SIMPLE: OSScreenInit completed\n");
    
    // Get buffer sizes
    uint32_t tvBufferSize = OSScreenGetBufferSizeEx(SCREEN_TV);
    uint32_t drcBufferSize = OSScreenGetBufferSizeEx(SCREEN_DRC);
    printf("SIMPLE: Buffer sizes - TV: %u, DRC: %u\n", tvBufferSize, drcBufferSize);
    
    if (tvBufferSize == 0 || drcBufferSize == 0) {
        printf("SIMPLE: Invalid buffer sizes\n");
        return -1;
    }
    
    // Allocate buffers
    void* tvBuffer = memalign(0x100, (size_t)tvBufferSize);
    void* drcBuffer = memalign(0x100, (size_t)drcBufferSize);
    printf("SIMPLE: Buffers allocated\n");
    
    if (!tvBuffer || !drcBuffer) {
        printf("SIMPLE: Buffer allocation failed\n");
        if (tvBuffer) free(tvBuffer);
        if (drcBuffer) free(drcBuffer);
        return -1;
    }
    
    // Set buffers
    OSScreenSetBufferEx(SCREEN_TV, tvBuffer);
    OSScreenSetBufferEx(SCREEN_DRC, drcBuffer);
    printf("SIMPLE: Buffers set\n");
    
    // Show red screen
    OSScreenClearBufferEx(SCREEN_TV, 0x00FF0000); // Red background
    OSScreenClearBufferEx(SCREEN_DRC, 0x00FF0000); // Red background
    OSScreenFlipBuffersEx(SCREEN_TV);
    OSScreenFlipBuffersEx(SCREEN_DRC);
    printf("SIMPLE: Red screen displayed\n");
    
    // Wait longer
    for (volatile int i = 0; i < 10000000; ++i) {
        // Longer delay loop
    }
    printf("SIMPLE: Delay completed\n");
    
    // Cleanup
    if (tvBuffer) free(tvBuffer);
    if (drcBuffer) free(drcBuffer);
    OSScreenShutdown();
    printf("SIMPLE: Shutdown completed\n");
    
    return 0;
}

#endif
