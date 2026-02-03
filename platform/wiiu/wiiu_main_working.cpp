/**
 * Wii U working test - using known working headers
 */

#include <coreinit/screen.h>
#include <coreinit/memory.h>
#include <malloc.h>
#include <cstdio>

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    
    printf("WORKING: Test started\n");
    
    // Try basic OSScreen
    OSScreenInit();
    printf("WORKING: OSScreenInit completed\n");
    
    printf("WORKING: Test completed successfully\n");
    
    return 0;
}
