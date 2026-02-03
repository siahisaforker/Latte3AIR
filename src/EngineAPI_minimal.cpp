/**
 * Wii U minimal EngineAPI implementation
 * Completely avoids the original EngineAPI header and implementation
 */

#if defined(PLATFORM_WIIU) || defined(__WIIU__)

// Minimal class definition to avoid including the original header
class EngineAPI {
public:
    EngineAPI() {}
    ~EngineAPI() {}
    
    bool initialize() { return true; }
    void runFrame() {}
};

#else

// Include original for other platforms
#include "EngineAPI.h"

#endif
