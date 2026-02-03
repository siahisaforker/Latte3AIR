/**
 * Wii U essentials test - only core systems, no platform files
 * Tests which specific platform file is causing the crash
 */

#if defined(PLATFORM_WIIU) || defined(__WIIU__)

#include <coreinit/screen.h>
#include <coreinit/memory.h>
#include <coreinit/filesystem.h>
#include <malloc.h>
#include <cstdio>

// Standard WUT entry point
extern "C" int main(int argc, char** argv) {
    (void)argc;
    (void)argv;
    
    printf("DEBUG: Starting essentials test\n");
    
    // Initialize basic Wii U systems
    FSInit();
    printf("DEBUG: FSInit completed\n");
    
    // Try to initialize OSScreen
    OSScreenInit();
    printf("DEBUG: OSScreenInit completed\n");
    
    // Get buffer sizes
    uint32_t tvBufferSize = OSScreenGetBufferSizeEx(SCREEN_TV);
    uint32_t drcBufferSize = OSScreenGetBufferSizeEx(SCREEN_DRC);
    printf("DEBUG: Buffer sizes - TV: %u, DRC: %u\n", tvBufferSize, drcBufferSize);
    
    if (tvBufferSize == 0 || drcBufferSize == 0) {
        printf("DEBUG: Invalid buffer sizes, exiting\n");
        OSScreenShutdown();
        return -1;
    }
    
    // Allocate buffers
    void* tvBuffer = memalign(0x100, (size_t)tvBufferSize);
    void* drcBuffer = memalign(0x100, (size_t)drcBufferSize);
    printf("DEBUG: Buffers allocated - TV: %p, DRC: %p\n", tvBuffer, drcBuffer);
    
    if (!tvBuffer || !drcBuffer) {
        printf("DEBUG: Buffer allocation failed\n");
        OSScreenShutdown();
        if (tvBuffer) free(tvBuffer);
        if (drcBuffer) free(drcBuffer);
        return -1;
    }
    
    // Set buffers
    OSScreenSetBufferEx(SCREEN_TV, tvBuffer);
    OSScreenSetBufferEx(SCREEN_DRC, drcBuffer);
    printf("DEBUG: Buffers set\n");
    
    // Show success - purple screen
    OSScreenClearBufferEx(SCREEN_TV, 0x00FF00FF); // Purple background
    OSScreenClearBufferEx(SCREEN_DRC, 0x00FF00FF); // Purple background
    OSScreenFlipBuffersEx(SCREEN_TV);
    OSScreenFlipBuffersEx(SCREEN_DRC);
    printf("DEBUG: Screen cleared and flipped\n");
    
    // Wait a bit
    for (volatile int i = 0; i < 5000000; ++i) {
        // Simple delay loop
    }
    printf("DEBUG: Delay completed\n");
    
    // Cleanup
    printf("DEBUG: Starting cleanup\n");
    if (tvBuffer) free(tvBuffer);
    if (drcBuffer) free(drcBuffer);
    OSScreenShutdown();
    printf("DEBUG: Shutdown completed\n");
    
    return 0;
}

#endif
