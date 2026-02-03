/**
 * Wii U minimal threading implementation
 * Only provides essential functions to avoid crashes
 */

#if defined(PLATFORM_WIIU) || defined(__WIIU__)

#include <coreinit/thread.h>
#include <coreinit/time.h>
#include <coreinit/mutex.h>
#include <coreinit/condition.h>
#include <cstdint>

namespace wiiu {

uint64_t getCurrentTime() {
    return OSGetSystemTick();
}

double getTimeInSeconds() {
    uint64_t ticks = OSGetSystemTick();
    return ticks / 40500000.0; // Wii U timer frequency (40.5MHz)
}

void sleep(uint32_t milliseconds) {
    OSSleepTicks(OSMillisecondsToTicks(milliseconds));
}

// Minimal thread handle - just a pointer
typedef void* ThreadHandle;

// Minimal thread creation
ThreadHandle createThread(void (*func)(void*), void* arg) {
    // For now, return null - threading not essential for basic functionality
    return nullptr;
}

void startThread(ThreadHandle thread) {
    // Not implemented for minimal version
}

void joinThread(ThreadHandle thread) {
    // Not implemented for minimal version
}

void destroyThread(ThreadHandle thread) {
    // Not implemented for minimal version
}

// Minimal mutex handle - just a pointer
typedef void* MutexHandle;

MutexHandle createMutex() {
    // Create a simple mutex
    OSMutex* mutex = new OSMutex();
    OSInitMutex(mutex);
    return mutex;
}

void lockMutex(MutexHandle mutex) {
    if (mutex) {
        OSLockMutex(static_cast<OSMutex*>(mutex));
    }
}

void unlockMutex(MutexHandle mutex) {
    if (mutex) {
        OSUnlockMutex(static_cast<OSMutex*>(mutex));
    }
}

void destroyMutex(MutexHandle mutex) {
    if (mutex) {
        OSMutex* osMutex = static_cast<OSMutex*>(mutex);
        // OSDestroyMutex doesn't exist, just delete
        delete osMutex;
    }
}

// Minimal condition handle - just a pointer
typedef void* ConditionHandle;

ConditionHandle createCondition() {
    // Create a simple condition variable
    OSCondition* condition = new OSCondition();
    // OSInitCondition doesn't exist, just return
    return condition;
}

void waitCondition(ConditionHandle cond, MutexHandle mutex) {
    if (cond && mutex) {
        OSWaitCond(static_cast<OSCondition*>(cond), static_cast<OSMutex*>(mutex));
    }
}

void signalCondition(ConditionHandle cond) {
    if (cond) {
        OSSignalCond(static_cast<OSCondition*>(cond));
    }
}

void broadcastCondition(ConditionHandle cond) {
    if (cond) {
        // OSBroadcastCond doesn't exist, just signal
        OSSignalCond(static_cast<OSCondition*>(cond));
    }
}

void destroyCondition(ConditionHandle cond) {
    if (cond) {
        OSCondition* osCondition = static_cast<OSCondition*>(cond);
        // OSDestroyCondition doesn't exist, just delete
        delete osCondition;
    }
}

} // namespace wiiu

#endif
