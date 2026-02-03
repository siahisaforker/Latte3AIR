#pragma once

#include <cstdint>
#include <functional>

#ifdef __cplusplus
extern "C" {
#endif

// OSThread types and functions
typedef struct OSThread OSThread;
typedef struct OSMutex OSMutex;
typedef struct OSCondition OSCondition;

typedef void* (*OSThreadEntryPoint)(void* arg);

// Thread functions
OSThread* OSCreateThread(OSThreadEntryPoint entry, void* arg, int32_t stackSize, int32_t priority, void* stack);
void OSDestroyThread(OSThread* thread);
void OSResumeThread(OSThread* thread);
void OSSuspendThread(OSThread* thread);
void OSJoinThread(OSThread* thread, void** result);
void OSThreadYield(void);

// Mutex functions
OSMutex* OSCreateMutex(void);
void OSDestroyMutex(OSMutex* mutex);
void OSLockMutex(OSMutex* mutex);
void OSUnlockMutex(OSMutex* mutex);
bool OSTryLockMutex(OSMutex* mutex);

// Condition variable functions  
OSCondition* OSCreateCondition(void);
void OSDestroyCondition(OSCondition* condition);
void OSSignalCondition(OSCondition* condition);
void OSWaitCondition(OSCondition* condition, OSMutex* mutex);

#ifdef __cplusplus
}
#endif

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

private:
    static void threadEntryPoint(void* arg);
    
    void startInternal();
    
    OSThread* mThread;
    ThreadFunction mFunction;
    bool mJoinable;
    
    static constexpr int32_t kDefaultStackSize = 64 * 1024; // 64KB
    static constexpr int32_t kDefaultPriority = 0;
};

} // namespace rmx
