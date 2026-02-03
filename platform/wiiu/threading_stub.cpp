/**
 * Wii U stub threading implementation
 * Provides only basic stub functions to avoid crashes
 */

#if defined(PLATFORM_WIIU) || defined(__WIIU__)

#include <coreinit/thread.h>
#include <coreinit/time.h>
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

// Stub thread handle - just a pointer
typedef void* ThreadHandle;

// Stub thread creation - returns null (no actual threads)
ThreadHandle createThread(void (*func)(void*), void* arg) {
    (void)func;
    (void)arg;
    return nullptr;
}

void startThread(ThreadHandle thread) {
    (void)thread;
    // No-op for stub
}

void joinThread(ThreadHandle thread) {
    (void)thread;
    // No-op for stub
}

void destroyThread(ThreadHandle thread) {
    (void)thread;
    // No-op for stub
}

// Stub mutex handle - just a pointer
typedef void* MutexHandle;

MutexHandle createMutex() {
    // Return a dummy handle
    return (void*)0x12345678;
}

void lockMutex(MutexHandle mutex) {
    (void)mutex;
    // No-op for stub
}

void unlockMutex(MutexHandle mutex) {
    (void)mutex;
    // No-op for stub
}

void destroyMutex(MutexHandle mutex) {
    (void)mutex;
    // No-op for stub
}

// Stub condition handle - just a pointer
typedef void* ConditionHandle;

ConditionHandle createCondition() {
    // Return a dummy handle
    return (void*)0x87654321;
}

void waitCondition(ConditionHandle cond, MutexHandle mutex) {
    (void)cond;
    (void)mutex;
    // No-op for stub
}

void signalCondition(ConditionHandle cond) {
    (void)cond;
    // No-op for stub
}

void broadcastCondition(ConditionHandle cond) {
    (void)cond;
    // No-op for stub
}

void destroyCondition(ConditionHandle cond) {
    (void)cond;
    // No-op for stub
}

} // namespace wiiu

#endif
