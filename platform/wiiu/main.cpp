#include <coreinit/screen.h>
#include <coreinit/memory.h>
#include <malloc.h>
#include <cstdio>

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    
    printf("TEST: Application started\n");
    
    // Initialize screen
    OSScreenInit();
    printf("TEST: OSScreen initialized\n");
    
    // Get buffer sizes
    uint32_t tvBufferSize = OSScreenGetBufferSizeEx(SCREEN_TV);
    uint32_t drcBufferSize = OSScreenGetBufferSizeEx(SCREEN_DRC);
    printf("TEST: TV buffer: %u, DRC buffer: %u\n", tvBufferSize, drcBufferSize);
    
    if (tvBufferSize > 0 && drcBufferSize > 0) {
        // Allocate buffers
        void* tvBuffer = memalign(0x100, tvBufferSize);
        void* drcBuffer = memalign(0x100, drcBufferSize);
        
        if (tvBuffer && drcBuffer) {
            printf("TEST: Buffers allocated successfully\n");
            
            // Set buffers
            OSScreenSetBufferEx(SCREEN_TV, tvBuffer);
            OSScreenSetBufferEx(SCREEN_DRC, drcBuffer);
            
            // Clear to green
            OSScreenClearBufferEx(SCREEN_TV, 0x0000FF00);
            OSScreenClearBufferEx(SCREEN_DRC, 0x0000FF00);
            
            // Flip buffers
            OSScreenFlipBuffersEx(SCREEN_TV);
            OSScreenFlipBuffersEx(SCREEN_DRC);
            
            printf("TEST: Green screen displayed\n");
            
            // Wait
            for (volatile int i = 0; i < 10000000; ++i);
            
            printf("TEST: Test completed\n");
            
            // Cleanup
            free(tvBuffer);
            free(drcBuffer);
        } else {
            printf("TEST: Buffer allocation failed\n");
        }
    } else {
        printf("TEST: Invalid buffer sizes\n");
    }
    
    OSScreenShutdown();
    printf("TEST: Shutdown complete\n");
    
    return 0;
}
