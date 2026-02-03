/**
 * Wii U minimal test - absolute bare minimum to get any screen output
 * Bypasses all complex initialization, tries to show a simple color
 */

#if defined(PLATFORM_WIIU) || defined(__WIIU__)

// Minimal includes only
#include <coreinit/screen.h>
#include <coreinit/memory.h>
#include <malloc.h>

// Global variables to avoid stack issues
static void* gTvBuffer = nullptr;
static void* gDrcBuffer = nullptr;

// Simple entry point - no complex initialization
int main(int argc, char** argv) {
    (void)argc;
    (void)argv;
    
    // Try the absolute minimum to show anything on screen
    // Initialize OSScreen
    OSScreenInit();
    
    // Get buffer sizes
    uint32_t tvBufferSize = OSScreenGetBufferSizeEx(0); // SCREEN_TV = 0
    uint32_t drcBufferSize = OSScreenGetBufferSizeEx(1); // SCREEN_DRC = 1
    
    // Allocate buffers with minimal alignment
    if (tvBufferSize > 0) {
        gTvBuffer = memalign(0x40, tvBufferSize);
    }
    if (drcBufferSize > 0) {
        gDrcBuffer = memalign(0x40, drcBufferSize);
    }
    
    // Set buffers if allocation succeeded
    if (gTvBuffer && tvBufferSize > 0) {
        OSScreenSetBufferEx(0, gTvBuffer);
    }
    if (gDrcBuffer && drcBufferSize > 0) {
        OSScreenSetBufferEx(1, gDrcBuffer);
    }
    
    // Clear screens to different colors to see if anything works
    if (gTvBuffer) {
        OSScreenClearBufferEx(0, 0x00FF0000); // Red on TV
    }
    if (gDrcBuffer) {
        OSScreenClearBufferEx(1, 0x0000FF00); // Green on DRC
    }
    
    // Flip buffers
    OSScreenFlipBuffersEx(0);
    OSScreenFlipBuffersEx(1);
    
    // Simple delay loop
    for (volatile int i = 0; i < 10000000; ++i) {
        // Wait
    }
    
    // Cleanup
    if (gTvBuffer) {
        free(gTvBuffer);
    }
    if (gDrcBuffer) {
        free(gDrcBuffer);
    }
    
    return 0;
}

#endif
