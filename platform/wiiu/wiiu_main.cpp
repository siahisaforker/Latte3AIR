/**
 * Wii U platform entry point.
 * OSInit, heap, FS, controller, renderer init (GX2 then OSScreen fallback), engine mainLoop.
 * No launcher logic; Aroma handles launch.
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

static bool gShouldQuit = false;

static void initOS() {
    // CoreInit is handled automatically by WUT runtime
}

static void initHeap() {
    // Heap is handled automatically by WUT runtime
}

static void initFS() {
    FSInit();
}

static void initController() {
    VPADInit();
    VPADStatus status;
    VPADRead(VPAD_CHAN_0, &status, 1, nullptr);
}

static uint32_t procUICallback(void* userdata) {
    (void)userdata;
    gShouldQuit = true;
    return 0;
}

static void initSysApp() {
    ProcUIRegisterCallback(PROCUI_CALLBACK_EXIT, procUICallback, nullptr, 0);
}

static void handleHomeButton() {
    VPADStatus status;
    VPADReadError error;
    if (VPADRead(VPAD_CHAN_0, &status, 1, &error) == VPAD_READ_SUCCESS) {
        if (status.hold & VPAD_BUTTON_HOME) {
            gShouldQuit = true;
        }
    }
}

static wiiu::Renderer* selectRenderer() {
    wiiu::Renderer* renderer = new wiiu::GX2Renderer();
    if (!renderer->init()) {
        delete renderer;
        renderer = new wiiu::OSScreenRenderer();
        renderer->init();
    }
    return renderer;
}

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;
    
    // Initialize basic Wii U systems
    FSInit();
    
    // Try to initialize OSScreen for a simple test
    OSScreenInit();
    
    // Get buffer sizes
    uint32_t tvBufferSize = OSScreenGetBufferSizeEx(SCREEN_TV);
    uint32_t drcBufferSize = OSScreenGetBufferSizeEx(SCREEN_DRC);
    
    // Allocate buffers
    void* tvBuffer = memalign(0x100, tvBufferSize);
    void* drcBuffer = memalign(0x100, drcBufferSize);
    
    if (tvBuffer && drcBuffer) {
        // Set buffers
        OSScreenSetBufferEx(SCREEN_TV, tvBuffer);
        OSScreenSetBufferEx(SCREEN_DRC, drcBuffer);
        
        // Show error message - NO ROM FOUND
        OSScreenClearBufferEx(SCREEN_TV, 0x00FF0000); // Red background
        OSScreenClearBufferEx(SCREEN_DRC, 0x00FF0000); // Red background
        
        // Flip buffers to show error
        OSScreenFlipBuffersEx(SCREEN_TV);
        OSScreenFlipBuffersEx(SCREEN_DRC);
        
        // Wait a bit (simple delay)
        for (volatile int i = 0; i < 5000000; ++i) {
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
    
    return 0;
}

#endif
