#include <coreinit/screen.h>
#include <coreinit/memory.h>
#include <malloc.h>

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    
    // Initialize screen
    OSScreenInit();
    
    // Get buffer sizes
    uint32_t tvBufferSize = OSScreenGetBufferSizeEx(SCREEN_TV);
    uint32_t drcBufferSize = OSScreenGetBufferSizeEx(SCREEN_DRC);
    
    if (tvBufferSize > 0 && drcBufferSize > 0) {
        // Allocate buffers
        void* tvBuffer = memalign(0x100, tvBufferSize);
        void* drcBuffer = memalign(0x100, drcBufferSize);
        
        if (tvBuffer && drcBuffer) {
            // Set buffers
            OSScreenSetBufferEx(SCREEN_TV, tvBuffer);
            OSScreenSetBufferEx(SCREEN_DRC, drcBuffer);
            
            // Show BLUE screen (different from all previous tests)
            OSScreenClearBufferEx(SCREEN_TV, 0x000000FF);
            OSScreenClearBufferEx(SCREEN_DRC, 0x000000FF);
            
            // Flip buffers
            OSScreenFlipBuffersEx(SCREEN_TV);
            OSScreenFlipBuffersEx(SCREEN_DRC);
            
            // Wait longer (10 seconds)
            for (volatile int i = 0; i < 20000000; ++i);
            
            // Show RED screen (second phase)
            OSScreenClearBufferEx(SCREEN_TV, 0x00FF0000);
            OSScreenClearBufferEx(SCREEN_DRC, 0x00FF0000);
            
            // Flip buffers
            OSScreenFlipBuffersEx(SCREEN_TV);
            OSScreenFlipBuffersEx(SCREEN_DRC);
            
            // Wait longer (10 seconds)
            for (volatile int i = 0; i < 20000000; ++i);
            
            // Cleanup
            free(tvBuffer);
            free(drcBuffer);
        }
    }
    
    OSScreenShutdown();
    
    return 0;
}
