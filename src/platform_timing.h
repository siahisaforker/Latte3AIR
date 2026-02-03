#pragma once

#if defined(PLATFORM_WIIU) || defined(__WIIU__)

#include <cstdint>

namespace platform_timing {

// Timer initialization
bool initializeTimer();
void shutdownTimer();

// Basic timing functions
uint32_t getTicks();
void delay(uint32_t ms);

// High-precision timing
uint64_t getCurrentTime();
double getTimeInSeconds();

// Delta time calculation with clamping
float getDeltaTime();
float clampDeltaTime(float deltaTime);

// Frame rate control
void frameRateLimit(int targetFPS);

// Performance monitoring class
class FrameTimer {
private:
    uint64_t mStartTime;
    uint64_t mFrameCount;
    float mTotalTime;
    
public:
    FrameTimer();
    
    void start();
    void beginFrame();
    void endFrame();
    
    float getAverageFrameTime() const;
    float getAverageFPS() const;
    uint64_t getFrameCount() const;
};

// Global frame timer functions
FrameTimer& getGlobalFrameTimer();
void startGlobalFrameTimer();
void beginGlobalFrame();
void endGlobalFrame();
float getAverageFrameTime();
float getAverageFPS();
uint64_t getGlobalFrameCount();

} // namespace platform_timing

#endif // PLATFORM_WIIU
