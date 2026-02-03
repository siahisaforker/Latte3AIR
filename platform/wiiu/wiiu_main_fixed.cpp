/**
 * Wii U platform entry point - using standard WUT entry point
 * Fixed version that doesn't use custom crt0
 */

#if defined(PLATFORM_WIIU) || defined(__WIIU__)

#include "render/renderer.h"
#include "render/gx2_renderer.h"
#include "render/osscreen_renderer.h"
#include "render/renderer_instance.h"
#include "../src/EngineAPI.h"
#include "input/input_state.h"

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

static uint32_t procUICallback(void* userdata) {
    (void)userdata;
    return 0;
}

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
    
    // Check for ROM file
    const char* romPath = "/vol/external01/sonic3air/rom/Sonic_Knuckles_wSonic3.bin";
    bool romExists = false;
    
    // Try to open ROM file
    FILE* romFile = fopen(romPath, "rb");
    if (romFile) {
        romExists = true;
        fclose(romFile);
    }
    
    if (!romExists) {
        // Show "No rom file found" message - simplified to avoid crashes
        OSScreenClearBufferEx(SCREEN_TV, 0x00FF0000); // Red background
        OSScreenClearBufferEx(SCREEN_DRC, 0x00FF0000); // Red background
        
        // Flip buffers to show error
        OSScreenFlipBuffersEx(SCREEN_TV);
        OSScreenFlipBuffersEx(SCREEN_DRC);
        
        // Wait longer to show the error message
        for (volatile int i = 0; i < 10000000; ++i) {
            // Simple delay loop
        }
    } else {
        // ROM found - show green screen briefly
        OSScreenClearBufferEx(SCREEN_TV, 0x0000FF00); // Green background
        OSScreenClearBufferEx(SCREEN_DRC, 0x0000FF00); // Green background
        OSScreenFlipBuffersEx(SCREEN_TV);
        OSScreenFlipBuffersEx(SCREEN_DRC);
        
        // Brief delay
        for (volatile int i = 0; i < 2000000; ++i) {
            // Simple delay loop
        }
        
        // Clear to blue before exit
        OSScreenClearBufferEx(SCREEN_TV, 0x000000FF); // Blue
        OSScreenClearBufferEx(SCREEN_DRC, 0x000000FF); // Blue
        OSScreenFlipBuffersEx(SCREEN_TV);
        OSScreenFlipBuffersEx(SCREEN_DRC);
    }
    
    // Cleanup
    if (tvBuffer) free(tvBuffer);
    if (drcBuffer) free(drcBuffer);
    
    OSScreenShutdown();
    
    return 0;
}

#endif
