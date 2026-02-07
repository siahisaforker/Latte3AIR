#pragma once

#include <cstdint>
#include <functional>

// When building with devkitPro/WUT, include official coreinit headers.
#ifdef __WUT__
#include <coreinit/thread.h>
#include <coreinit/mutex.h>
#include <coreinit/condition.h>
#include <coreinit/time.h>
#else
#ifdef __cplusplus
extern "C" {
#endif

// Minimal fallback declarations (only to satisfy compilation on non-WUT builds).
typedef struct OSThread OSThread;
typedef struct OSMutex OSMutex;
typedef struct OSCondition OSCondition;

typedef void* (*OSThreadEntryPoint)(void* arg);

// Thread functions (fallback signatures)
OSThread* OSCreateThread(OSThreadEntryPoint entry, void* arg, int32_t stackSize, int32_t priority, void* stack);
void OSDestroyThread(OSThread* thread);
void OSResumeThread(OSThread* thread);
void OSSuspendThread(OSThread* thread);
void OSJoinThread(OSThread* thread, void** result);
void OSThreadYield(void);

// Mutex functions (fallback signatures)
OSMutex* OSCreateMutex(void);
void OSDestroyMutex(OSMutex* mutex);
void OSLockMutex(OSMutex* mutex);
void OSUnlockMutex(OSMutex* mutex);
bool OSTryLockMutex(OSMutex* mutex);

// Condition variable functions (fallback signatures)
OSCondition* OSCreateCondition(void);
void OSDestroyCondition(OSCondition* condition);
void OSSignalCondition(OSCondition* condition);
void OSWaitCondition(OSCondition* condition, OSMutex* mutex);

#ifdef __cplusplus
}
#endif
#endif // __WUT__

namespace rmx {

class WiiUMutex
{
public:
    WiiUMutex();
    ~WiiUMutex();
    
    void lock();
    void unlock();
    bool tryLock();
    
    OSMutex* getNativeMutex() { return mMutex; }

private:
    OSMutex* mMutex;
};

class WiiUConditionVariable
{
public:
    WiiUConditionVariable();
    ~WiiUConditionVariable();
    
    void signal();
    void wait(WiiUMutex& mutex);
    
    OSCondition* getNativeCondition() { return mCondition; }

private:
    OSCondition* mCondition;
};

class WiiUThread
{
public:
    using ThreadFunction = std::function<void()>;
    
    WiiUThread();
    ~WiiUThread();
    
    template<typename Func, typename... Args>
    void start(Func&& func, Args&&... args)
    {
        mFunction = std::bind(std::forward<Func>(func), std::forward<Args>(args)...);
        startInternal();
    }
    
    void join();
    void detach();
    bool joinable() const;
    
    static void yield();
    static void sleep(uint32_t milliseconds);

    // Called by the thread wrapper to execute the stored function.
    void runThreadFunction();

private:
    static void threadEntryPoint(void* arg);
    
    void startInternal();
    
    OSThread* mThread;
    void* mStack;
    ThreadFunction mFunction;
    bool mJoinable;
    
    static constexpr int32_t kDefaultStackSize = 128 * 1024; // 128KB — safe for deep call stacks
    static constexpr int32_t kDefaultPriority = 16;          // Lower than default (0) to avoid starving main thread
};

} // namespace rmx
