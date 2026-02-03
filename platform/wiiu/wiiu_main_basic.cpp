/**
 * Wii U basic test - no Wii U APIs at all, just to test if the executable runs
 */

#if defined(PLATFORM_WIIU) || defined(__WIIU__)

// No includes - just basic C++

// Standard WUT entry point
extern "C" int main(int argc, char** argv) {
    (void)argc;
    (void)argv;
    
    // Just return immediately - no API calls
    return 42;
}

#endif
