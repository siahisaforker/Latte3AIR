/**
 * Wii U debug test - minimal version to isolate crashes
 * Displays "game initiated" before any complex operations
 */

#if defined(PLATFORM_WIIU) || defined(__WIIU__)

#include <coreinit/core.h>
#include <coreinit/memory.h>
#include <coreinit/filesystem.h>
#include <coreinit/systeminfo.h>
#include <coreinit/screen.h>
#include <vpad/input.h>
#include <sysapp/switch.h>
#include <proc_ui/procui.h>
#include <cstdlib>
#include <cstring>
#include <chrono>
#include <thread>
#include <malloc.h>
#include <cstdio>

static bool gShouldQuit = false;

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;
    
    // DEBUG: Print to console first
    printf("DEBUG: Game initiated\\n");
    
    // Initialize basic Wii U systems
    printf("DEBUG: Initializing core systems...\\n");
    FSInit();
    
    // Try to initialize OSScreen
    printf("DEBUG: Initializing OSScreen...\\n");
    OSScreenInit();
    
    // Get buffer sizes
    printf("DEBUG: Getting buffer sizes...\\n");
    uint32_t tvBufferSize = OSScreenGetBufferSizeEx(SCREEN_TV);
    uint32_t drcBufferSize = OSScreenGetBufferSizeEx(SCREEN_DRC);
    
    printf("DEBUG: TV buffer size: %u, DRC buffer size: %u\\n", tvBufferSize, drcBufferSize);
    
    if (tvBufferSize == 0 || drcBufferSize == 0) {
        printf("DEBUG: Invalid buffer sizes, exiting\\n");
        OSScreenShutdown();
        return -1;
    }
    
    // Allocate buffers
    printf("DEBUG: Allocating buffers...\\n");
    void* tvBuffer = memalign(0x100, (size_t)tvBufferSize);
    void* drcBuffer = memalign(0x100, (size_t)drcBufferSize);
    
    if (!tvBuffer || !drcBuffer) {
        printf("DEBUG: Buffer allocation failed, exiting\\n");
        OSScreenShutdown();
        if (tvBuffer) free(tvBuffer);
        if (drcBuffer) free(drcBuffer);
        return -1;
    }
    
    printf("DEBUG: Buffers allocated successfully\\n");
    
    // Set buffers
    printf("DEBUG: Setting screen buffers...\\n");
    OSScreenSetBufferEx(SCREEN_TV, tvBuffer);
    OSScreenSetBufferEx(SCREEN_DRC, drcBuffer);
    
    // Show debug message on screen
    printf("DEBUG: Displaying debug message...\\n");
    OSScreenClearBufferEx(SCREEN_TV, 0x0000FF00); // Green background
    OSScreenClearBufferEx(SCREEN_DRC, 0x0000FF00); // Green background
    
    // Flip buffers to show debug message
    printf("DEBUG: Flipping buffers...\\n");
    OSScreenFlipBuffersEx(SCREEN_TV);
    OSScreenFlipBuffersEx(SCREEN_DRC);
    
    printf("DEBUG: Debug test completed successfully\\n");
    
    // Wait a bit to see the message
    printf("DEBUG: Waiting 3 seconds...\\n");
    for (volatile int i = 0; i < 3000000; ++i) {
        // Simple delay loop (3 seconds)
    }
    
    printf("DEBUG: Cleaning up...\\n");
    
    // Cleanup
    if (tvBuffer) free(tvBuffer);
    if (drcBuffer) free(drcBuffer);
    
    OSScreenShutdown();
    
    printf("DEBUG: Exit\\n");
    return 0;
}

#endif
