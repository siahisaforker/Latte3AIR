// Wii U application entry point
void __attribute__((noreturn)) exit(int code) {
    (void)code;
    // Infinite loop - Wii U apps don't normally exit
    while (1) {
        // Wait forever
    }
}
