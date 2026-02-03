#pragma once

#if defined(PLATFORM_WIIU) || defined(__WIIU__)

#include <cstdint>

namespace platform_threading {

// Thread types
typedef void (*ThreadFunction)(void*);
typedef void* ThreadHandle;
typedef void* MutexHandle;
typedef void* ConditionHandle;

// Thread functions
ThreadHandle createThread(ThreadFunction func, void* arg, int priority = 0, uint32_t stackSize = 0x10000);
void startThread(ThreadHandle thread);
void joinThread(ThreadHandle thread);
void destroyThread(ThreadHandle thread);

// Mutex functions
MutexHandle createMutex();
void lockMutex(MutexHandle mutex);
void unlockMutex(MutexHandle mutex);
void destroyMutex(MutexHandle mutex);

// Condition variable functions
ConditionHandle createCondition();
void waitCondition(ConditionHandle cond, MutexHandle mutex);
void signalCondition(ConditionHandle cond);
void broadcastCondition(ConditionHandle cond);
void destroyCondition(ConditionHandle cond);

// Thread-safe wrapper classes
class Mutex {
private:
    MutexHandle mHandle;
    
public:
    Mutex();
    ~Mutex();
    
    void lock();
    void unlock();
    
    MutexHandle getHandle() const { return mHandle; }
};

class LockGuard {
private:
    Mutex& mMutex;
    
public:
    explicit LockGuard(Mutex& mutex);
    ~LockGuard();
    
    // Non-copyable
    LockGuard(const LockGuard&) = delete;
    LockGuard& operator=(const LockGuard&) = delete;
};

} // namespace platform_threading

#endif // PLATFORM_WIIU
