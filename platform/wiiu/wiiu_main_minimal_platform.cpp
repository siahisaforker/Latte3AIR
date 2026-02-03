/**
 * Wii U minimal platform test - only essential platform files
 * Tests which platform file is causing the crash
 */

#if defined(PLATFORM_WIIU) || defined(__WIIU__)

#include <coreinit/core.h>
#include <coreinit/memory.h>
#include <coreinit/filesystem.h>
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

// Standard WUT entry point
extern "C" int main(int argc, char** argv) {
    (void)argc;
    (void)argv;
    
    // Initialize basic Wii U systems
    FSInit();
    
    // Try to initialize OSScreen
    OSScreenInit();
    
    // Get buffer sizes
    uint32_t tvBufferSize = OSScreenGetBufferSizeEx(SCREEN_TV);
    uint32_t drcBufferSize = OSScreenGetBufferSizeEx(SCREEN_DRC);
    
    if (tvBufferSize == 0 || drcBufferSize == 0) {
        OSScreenShutdown();
        return -1;
    }
    
    // Allocate buffers
    void* tvBuffer = memalign(0x100, (size_t)tvBufferSize);
    void* drcBuffer = memalign(0x100, (size_t)drcBufferSize);
    
    if (!tvBuffer || !drcBuffer) {
        OSScreenShutdown();
        if (tvBuffer) free(tvBuffer);
        if (drcBuffer) free(drcBuffer);
        return -1;
    }
    
    // Set buffers
    OSScreenSetBufferEx(SCREEN_TV, tvBuffer);
    OSScreenSetBufferEx(SCREEN_DRC, drcBuffer);
    
    // Show success - green screen
    OSScreenClearBufferEx(SCREEN_TV, 0x0000FF00); // Green background
    OSScreenClearBufferEx(SCREEN_DRC, 0x0000FF00); // Green background
    OSScreenFlipBuffersEx(SCREEN_TV);
    OSScreenFlipBuffersEx(SCREEN_DRC);
    
    // Wait a bit
    for (volatile int i = 0; i < 5000000; ++i) {
        // Simple delay loop
    }
    
    // Cleanup
    if (tvBuffer) free(tvBuffer);
    if (drcBuffer) free(drcBuffer);
    
    OSScreenShutdown();
    
    return 0;
}

#endif
