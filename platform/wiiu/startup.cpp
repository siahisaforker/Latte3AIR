#include <coreinit/core.h>
#include <coreinit/systeminfo.h>

extern "C" int main(int argc, char** argv);

// Wii U application entry point
extern "C" void __start() {
    // Initialize core systems (WUT handles this automatically)
    
    // Call main
    int result = main(0, nullptr);
    
    // Exit (WUT handles this automatically)
    (void)result;
}
