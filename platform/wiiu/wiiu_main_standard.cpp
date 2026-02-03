/**
 * Wii U standard test - using standard WUT entry point
 * Tests if the issue is with custom crt0 assembly
 */

#if defined(PLATFORM_WIIU) || defined(__WIIU__)

// No includes - just basic C

// Use standard WUT entry point
extern "C" int main(int argc, char** argv) {
    (void)argc;
    (void)argv;
    
    // Just return immediately - no system calls
    return 0;
}

#endif
