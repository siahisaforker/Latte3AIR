/**
 * Wii U bare test - only main function, no other dependencies
 * Tests if the issue is with linking or with OSScreen itself
 */

#if defined(PLATFORM_WIIU) || defined(__WIIU__)

// Only the absolute minimum includes
#include <coreinit/screen.h>
#include <coreinit/memory.h>
#include <malloc.h>

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;
    
    // Try to initialize OSScreen - this is the bare minimum
    OSScreenInit();
    
    // Get buffer sizes
    uint32_t tvBufferSize = OSScreenGetBufferSizeEx(SCREEN_TV);
    
    // Try to clear and flip TV screen
    OSScreenClearBufferEx(SCREEN_TV, 0x00FF0000); // Red
    OSScreenFlipBuffersEx(SCREEN_TV);
    
    // Simple delay
    for (volatile int i = 0; i < 5000000; ++i) {}
    
    return 0;
}

#endif
