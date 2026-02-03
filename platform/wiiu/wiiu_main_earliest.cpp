/**
 * Wii U earliest test - no OSScreen, just basic execution test
 * Tests if we can get any code execution at all
 */

#if defined(PLATFORM_WIIU) || defined(__WIIU__)

// No includes at all - just basic C

// Global variable to test if code runs
volatile int gTestValue = 0;

// Simple function to test execution
void testFunction() {
    gTestValue = 123;
}

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;
    
    // Set test value to indicate main was reached
    gTestValue = 42;
    
    // Call a function to test basic execution
    testFunction();
    
    // Infinite loop to prevent exit
    while (1) {
        // Do nothing - just keep running
        gTestValue = gTestValue + 1;
    }
    
    return 0;
}

#endif
