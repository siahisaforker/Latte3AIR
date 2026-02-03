#include "platform_timing.h"

#if defined(PLATFORM_WIIU) || defined(__WIIU__)

#include "../platform/wiiu/threading.h"
#include <cstring>
#include <cmath>
#include <algorithm>
#include <cstdio>

// Complete timing implementation using Wii U system ticks
namespace platform_timing {

static bool sTimingInitialized = false;
static uint64_t sStartTime = 0;
static uint64_t sLastFrameTime = 0;

// Wii U timer frequency constants
static constexpr uint64_t WIIU_TIMER_FREQUENCY = 40500000; // 40.5 MHz
static constexpr double SECONDS_PER_TICK = 1.0 / WIIU_TIMER_FREQUENCY;
static constexpr double MILLISECONDS_PER_TICK = 1000.0 / WIIU_TIMER_FREQUENCY;

// High-resolution timer
class HighResTimer {
private:
    uint64_t mStartTime;
    uint64_t mPauseTime;
    bool mPaused;
    
public:
    HighResTimer() : mStartTime(0), mPauseTime(0), mPaused(false) {}
    
    void start() {
        mStartTime = getCurrentTime();
        mPaused = false;
        mPauseTime = 0;
    }
    
    void pause() {
        if (!mPaused) {
            mPauseTime = getCurrentTime();
            mPaused = true;
        }
    }
    
    void resume() {
        if (mPaused) {
            uint64_t pauseDuration = getCurrentTime() - mPauseTime;
            mStartTime += pauseDuration;
            mPaused = false;
            mPauseTime = 0;
        }
    }
    
    uint64_t getElapsedTicks() const {
        if (mPaused) {
            return mPauseTime - mStartTime;
        }
        return getCurrentTime() - mStartTime;
    }
    
    double getElapsedSeconds() const {
        return getElapsedTicks() * SECONDS_PER_TICK;
    }
    
    double getElapsedMilliseconds() const {
        return getElapsedTicks() * MILLISECONDS_PER_TICK;
    }
};

// Frame rate limiter
class FrameRateLimiter {
private:
    uint64_t mFrameInterval;
    uint64_t mLastFrameTime;
    uint64_t mAccumulator;
    uint32_t mTargetFPS;
    
public:
    FrameRateLimiter(uint32_t targetFPS) : mTargetFPS(targetFPS) {
        setFrameRate(targetFPS);
        mLastFrameTime = getCurrentTime();
        mAccumulator = 0;
    }
    
    void setFrameRate(uint32_t targetFPS) {
        mTargetFPS = std::max(1u, targetFPS);
        mFrameInterval = WIIU_TIMER_FREQUENCY / mTargetFPS;
    }
    
    void beginFrame() {
        mLastFrameTime = getCurrentTime();
    }
    
    void endFrame() {
        uint64_t currentTime = getCurrentTime();
        uint64_t frameTime = currentTime - mLastFrameTime;
        
        if (frameTime < mFrameInterval) {
            uint64_t sleepTime = mFrameInterval - frameTime;
            wiiu::sleep(static_cast<uint32_t>(sleepTime * MILLISECONDS_PER_TICK));
        }
        
        mAccumulator += frameTime;
    }
    
    uint32_t getTargetFPS() const {
        return mTargetFPS;
    }
    
    double getActualFPS() const {
        if (mAccumulator == 0) return 0.0;
        return static_cast<double>(WIIU_TIMER_FREQUENCY) / static_cast<double>(mAccumulator);
    }
    
    void reset() {
        mAccumulator = 0;
        mLastFrameTime = getCurrentTime();
    }
};

// Performance profiler
class PerformanceProfiler {
private:
    struct ProfileEntry {
        const char* name;
        uint64_t totalTime;
        uint64_t callCount;
        uint64_t minTime;
        uint64_t maxTime;
        bool active;
    };
    
    static constexpr size_t MAX_ENTRIES = 64;
    ProfileEntry mEntries[MAX_ENTRIES];
    size_t mEntryCount;
    HighResTimer mTimer;
    
public:
    PerformanceProfiler() : mEntryCount(0) {
        memset(mEntries, 0, sizeof(mEntries));
    }
    
    void beginProfile(const char* name) {
        if (!name) return;
        
        // Find or create entry
        size_t index = findEntry(name);
        if (index == MAX_ENTRIES) return; // Full
        
        mEntries[index].active = true;
        mTimer.start();
    }
    
    void endProfile(const char* name) {
        if (!name) return;
        
        size_t index = findEntry(name);
        if (index == MAX_ENTRIES) return;
        
        ProfileEntry& entry = mEntries[index];
        if (!entry.active) return;
        
        uint64_t elapsed = mTimer.getElapsedTicks();
        
        entry.totalTime += elapsed;
        entry.callCount++;
        
        if (elapsed < entry.minTime || entry.callCount == 1) {
            entry.minTime = elapsed;
        }
        
        if (elapsed > entry.maxTime) {
            entry.maxTime = elapsed;
        }
        
        entry.active = false;
    }
    
    void reset() {
        for (size_t i = 0; i < mEntryCount; ++i) {
            mEntries[i].totalTime = 0;
            mEntries[i].callCount = 0;
            mEntries[i].minTime = 0;
            mEntries[i].maxTime = 0;
            mEntries[i].active = false;
        }
    }
    
    void getStats(const char* name, double& avgMs, double& minMs, double& maxMs, uint64_t& callCount) {
        size_t index = findEntry(name);
        if (index == MAX_ENTRIES) {
            avgMs = minMs = maxMs = 0.0;
            callCount = 0;
            return;
        }
        
        const ProfileEntry& entry = mEntries[index];
        callCount = entry.callCount;
        
        if (callCount > 0) {
            avgMs = (entry.totalTime * MILLISECONDS_PER_TICK) / callCount;
            minMs = entry.minTime * MILLISECONDS_PER_TICK;
            maxMs = entry.maxTime * MILLISECONDS_PER_TICK;
        } else {
            avgMs = minMs = maxMs = 0.0;
        }
    }
    
private:
    size_t findEntry(const char* name) {
        for (size_t i = 0; i < mEntryCount; ++i) {
            if (strcmp(mEntries[i].name, name) == 0) {
                return i;
            }
        }
        
        if (mEntryCount < MAX_ENTRIES) {
            size_t index = mEntryCount++;
            mEntries[index].name = name;
            return index;
        }
        
        return MAX_ENTRIES;
    }
};

// Global instances
static FrameRateLimiter* gFrameLimiter = nullptr;
static PerformanceProfiler* gProfiler = nullptr;

// Timer initialization
bool initializeTimer() {
    if (sTimingInitialized) return true;
    
    sStartTime = getCurrentTime();
    sLastFrameTime = sStartTime;
    
    // Create global instances
    gFrameLimiter = new FrameRateLimiter(60); // Default 60 FPS
    gProfiler = new PerformanceProfiler();
    
    sTimingInitialized = true;
    return true;
}

void shutdownTimer() {
    if (!sTimingInitialized) return;
    
    delete gFrameLimiter;
    delete gProfiler;
    
    gFrameLimiter = nullptr;
    gProfiler = nullptr;
    
    sTimingInitialized = false;
}

// Basic timing functions
uint32_t getTicks() {
    return static_cast<uint32_t>(getCurrentTime() * MILLISECONDS_PER_TICK);
}

void delay(uint32_t ms) {
    uint64_t ticks = static_cast<uint64_t>(ms) * WIIU_TIMER_FREQUENCY / 1000;
    OSSleepTicks(ticks);
}

// High-precision timing
uint64_t getCurrentTime() {
    return OSGetSystemTick();
}

double getTimeInSeconds() {
    return static_cast<double>(getCurrentTime()) * SECONDS_PER_TICK;
}

// Delta time calculation with clamping
float getDeltaTime() {
    if (!sTimingInitialized) return 0.016f; // Default 60 FPS
    
    uint64_t currentTime = getCurrentTime();
    if (sLastFrameTime == 0) {
        sLastFrameTime = currentTime;
        return 0.016f;
    }
    
    double deltaTime = (currentTime - sLastFrameTime) * SECONDS_PER_TICK;
    sLastFrameTime = currentTime;
    
    // Clamp delta time to prevent physics explosions
    return clampDeltaTime(static_cast<float>(deltaTime));
}

float clampDeltaTime(float deltaTime) {
    // Clamp to reasonable range: 1ms to 100ms
    if (deltaTime <= 0.001f) return 0.001f;
    if (deltaTime >= 0.1f) return 0.1f;
    return deltaTime;
}

// Frame rate control
void frameRateLimit(int targetFPS) {
    if (targetFPS <= 0) return;
    
    if (!gFrameLimiter) {
        gFrameLimiter = new FrameRateLimiter(targetFPS);
    } else {
        gFrameLimiter->setFrameRate(targetFPS);
    }
    
    gFrameLimiter->beginFrame();
}

void endFrame() {
    if (gFrameLimiter) {
        gFrameLimiter->endFrame();
    }
}

float getAverageFrameTime() {
    if (!gFrameLimiter) return 0.016f;
    return 1.0f / gFrameLimiter->getActualFPS();
}

float getAverageFPS() {
    if (!gFrameLimiter) return 60.0f;
    return static_cast<float>(gFrameLimiter->getActualFPS());
}

// Performance monitoring
FrameTimer::FrameTimer() : mStartTime(0), mFrameCount(0), mTotalTime(0.0f) {
}

void FrameTimer::start() {
    mStartTime = getCurrentTime();
    mFrameCount = 0;
    mTotalTime = 0.0f;
}

void FrameTimer::beginFrame() {
    // Called at start of frame
}

void FrameTimer::endFrame() {
    // Called at end of frame
    if (mStartTime > 0) {
        uint64_t currentTime = getCurrentTime();
        float frameTime = static_cast<float>((currentTime - mStartTime) * SECONDS_PER_TICK);
        mTotalTime += frameTime;
        mFrameCount++;
    }
}

float FrameTimer::getAverageFrameTime() const {
    return (mFrameCount > 0) ? (mTotalTime / mFrameCount) : 0.016f;
}

float FrameTimer::getAverageFPS() const {
    float avgFrameTime = getAverageFrameTime();
    return (avgFrameTime > 0.0f) ? (1.0f / avgFrameTime) : 0.0f;
}

uint64_t FrameTimer::getFrameCount() const {
    return mFrameCount;
}

// Global frame timer instance
static FrameTimer sGlobalFrameTimer;

FrameTimer& getGlobalFrameTimer() {
    return sGlobalFrameTimer;
}

void startGlobalFrameTimer() {
    sGlobalFrameTimer.start();
}

void beginGlobalFrame() {
    sGlobalFrameTimer.beginFrame();
}

void endGlobalFrame() {
    sGlobalFrameTimer.endFrame();
    endFrame(); // Also call frame rate limiting
}

// Profiling functions
void beginProfile(const char* name) {
    if (gProfiler) {
        gProfiler->beginProfile(name);
    }
}

void endProfile(const char* name) {
    if (gProfiler) {
        gProfiler->endProfile(name);
    }
}

void getProfileStats(const char* name, double& avgMs, double& minMs, double& maxMs, uint64_t& callCount) {
    if (gProfiler) {
        gProfiler->getStats(name, avgMs, minMs, maxMs, callCount);
    } else {
        avgMs = minMs = maxMs = 0.0;
        callCount = 0;
    }
}

void resetProfiler() {
    if (gProfiler) {
        gProfiler->reset();
    }
}

// Utility functions
uint64_t ticksToMilliseconds(uint64_t ticks) {
    return ticks * 1000 / WIIU_TIMER_FREQUENCY;
}

uint64_t millisecondsToTicks(uint64_t milliseconds) {
    return milliseconds * WIIU_TIMER_FREQUENCY / 1000;
}

double ticksToSeconds(uint64_t ticks) {
    return static_cast<double>(ticks) / WIIU_TIMER_FREQUENCY;
}

uint64_t secondsToTicks(double seconds) {
    return static_cast<uint64_t>(seconds * WIIU_TIMER_FREQUENCY);
}

// Time formatting
void formatTime(uint64_t ticks, char* buffer, size_t bufferSize) {
    if (!buffer || bufferSize == 0) return;
    
    double seconds = ticksToSeconds(ticks);
    uint64_t hours = static_cast<uint64_t>(seconds) / 3600;
    uint64_t minutes = (static_cast<uint64_t>(seconds) % 3600) / 60;
    uint64_t secs = static_cast<uint64_t>(seconds) % 60;
    uint64_t ms = static_cast<uint64_t>((seconds - static_cast<uint64_t>(seconds)) * 1000);
    
    if (hours > 0) {
        snprintf(buffer, bufferSize, "%02llu:%02llu:%02llu.%03llu", hours, minutes, secs, ms);
    } else if (minutes > 0) {
        snprintf(buffer, bufferSize, "%02llu:%02llu.%03llu", minutes, secs, ms);
    } else {
        snprintf(buffer, bufferSize, "%02llu.%03llu", secs, ms);
    }
}

// Sleep with precision
void preciseSleep(double seconds) {
    if (seconds <= 0.0) return;
    
    uint64_t ticks = secondsToTicks(seconds);
    uint64_t startTime = getCurrentTime();
    
    while ((getCurrentTime() - startTime) < ticks) {
        // Yield to other threads
        OSSleepTicks(OSMillisecondsToTicks(1));
    }
}

} // namespace platform_timing

#endif // PLATFORM_WIIU
