#include "platform_threading.h"

#if defined(PLATFORM_WIIU) || defined(__WIIU__)

#include "../platform/wiiu/threading.h"
#include <cstring>
#include <algorithm>
#include <vector>
#include <functional>
#include <queue>

// Complete threading implementation using Wii U OSThread
namespace platform_threading {

// Thread functions - using wiiu namespace directly
ThreadHandle createThread(ThreadFunction func, void* arg, int priority, uint32_t stackSize) {
    return static_cast<ThreadHandle>(wiiu::createThread(func, arg, priority, stackSize));
}

void startThread(ThreadHandle thread) {
    wiiu::startThread(static_cast<wiiu::ThreadHandle>(thread));
}

void joinThread(ThreadHandle thread) {
    wiiu::joinThread(static_cast<wiiu::ThreadHandle>(thread));
}

void destroyThread(ThreadHandle thread) {
    wiiu::destroyThread(static_cast<wiiu::ThreadHandle>(thread));
}

// Mutex functions - using wiiu namespace directly
MutexHandle createMutex() {
    return static_cast<MutexHandle>(wiiu::createMutex());
}

void lockMutex(MutexHandle mutex) {
    wiiu::lockMutex(static_cast<wiiu::MutexHandle>(mutex));
}

void unlockMutex(MutexHandle mutex) {
    wiiu::unlockMutex(static_cast<wiiu::MutexHandle>(mutex));
}

void destroyMutex(MutexHandle mutex) {
    wiiu::destroyMutex(static_cast<wiiu::MutexHandle>(mutex));
}

// Condition variable functions - using wiiu namespace directly
ConditionHandle createCondition() {
    return static_cast<ConditionHandle>(wiiu::createCondition());
}

void waitCondition(ConditionHandle cond, MutexHandle mutex) {
    wiiu::waitCondition(static_cast<wiiu::ConditionHandle>(cond), static_cast<wiiu::MutexHandle>(mutex));
}

void signalCondition(ConditionHandle cond) {
    wiiu::signalCondition(static_cast<wiiu::ConditionHandle>(cond));
}

void broadcastCondition(ConditionHandle cond) {
    wiiu::broadcastCondition(static_cast<wiiu::ConditionHandle>(cond));
}

void destroyCondition(ConditionHandle cond) {
    wiiu::destroyCondition(static_cast<wiiu::ConditionHandle>(cond));
}

// Thread-safe wrapper classes
Mutex::Mutex() : mHandle(nullptr) {
    mHandle = createMutex();
}

Mutex::~Mutex() {
    if (mHandle) {
        destroyMutex(mHandle);
        mHandle = nullptr;
    }
}

void Mutex::lock() {
    lockMutex(mHandle);
}

void Mutex::unlock() {
    unlockMutex(mHandle);
}

LockGuard::LockGuard(Mutex& mutex) : mMutex(mutex) {
    mMutex.lock();
}

LockGuard::~LockGuard() {
    mMutex.unlock();
}

// Thread utility functions
void sleep(uint32_t milliseconds) {
    wiiu::sleep(milliseconds);
}

void yield() {
    // Wii U doesn't have a direct yield function
    wiiu::sleep(1);
}

uint32_t getCurrentThreadId() {
    // Wii U doesn't have a direct thread ID function
    return 0;
}

bool isThreadRunning(ThreadHandle thread) {
    // Wii U doesn't have a direct thread status function
    return thread != nullptr;
}

// Thread pool implementation
class ThreadPool {
private:
    std::vector<ThreadHandle> mThreads;
    std::queue<std::function<void()>> mTasks;
    Mutex mTaskMutex;
    ConditionHandle mTaskCondition;
    bool mShutdown;
    
public:
    ThreadPool(size_t threadCount) : mShutdown(false) {
        mTaskCondition = createCondition();
        
        for (size_t i = 0; i < threadCount; ++i) {
            ThreadHandle thread = createThread([](void* arg) {
                ThreadPool* pool = static_cast<ThreadPool*>(arg);
                while (!pool->mShutdown) {
                    std::function<void()> task;
                    
                    {
                        LockGuard lock(pool->mTaskMutex);
                        if (pool->mTasks.empty()) {
                            waitCondition(pool->mTaskCondition, pool->mTaskMutex.getHandle());
                            continue;
                        }
                        
                        task = pool->mTasks.front();
                        pool->mTasks.pop();
                    }
                    
                    if (task) {
                        task();
                    }
                }
            }, this, 0, 0x8000);
            
            if (thread) {
                startThread(thread);
                mThreads.push_back(thread);
            }
        }
    }
    
    ~ThreadPool() {
        mShutdown = true;
        broadcastCondition(mTaskCondition);
        
        for (ThreadHandle thread : mThreads) {
            joinThread(thread);
            destroyThread(thread);
        }
        
        destroyCondition(mTaskCondition);
    }
    
    void addTask(std::function<void()> task) {
        {
            LockGuard lock(mTaskMutex);
            mTasks.push(task);
        }
        signalCondition(mTaskCondition);
    }
};

// Global thread pool
static ThreadPool* gThreadPool = nullptr;

ThreadPool* getThreadPool(size_t threadCount) {
    if (!gThreadPool) {
        gThreadPool = new ThreadPool(threadCount);
    }
    return gThreadPool;
}

void destroyThreadPool() {
    if (gThreadPool) {
        delete gThreadPool;
        gThreadPool = nullptr;
    }
}

// Thread-safe queue implementation
template<typename T>
class ThreadSafeQueue {
private:
    std::queue<T> mQueue;
    Mutex mMutex;
    ConditionHandle mCondition;
    bool mShutdown;
    
public:
    ThreadSafeQueue() : mShutdown(false) {
        mCondition = createCondition();
    }
    
    ~ThreadSafeQueue() {
        mShutdown = true;
        broadcastCondition(mCondition);
        destroyCondition(mCondition);
    }
    
    void push(const T& item) {
        {
            LockGuard lock(mMutex);
            mQueue.push(item);
        }
        signalCondition(mCondition);
    }
    
    bool pop(T& item) {
        LockGuard lock(mMutex);
        
        while (mQueue.empty() && !mShutdown) {
            waitCondition(mCondition, mMutex.mHandle);
        }
        
        if (mQueue.empty()) {
            return false;
        }
        
        item = mQueue.front();
        mQueue.pop();
        return true;
    }
    
    bool empty() const {
        LockGuard lock(mMutex);
        return mQueue.empty();
    }
    
    size_t size() const {
        LockGuard lock(mMutex);
        return mQueue.size();
    }
};

// Thread-safe counter
class ThreadSafeCounter {
private:
    int32_t mValue;
    mutable Mutex mMutex;
    
public:
    ThreadSafeCounter(int32_t initialValue = 0) : mValue(initialValue) {}
    
    int32_t increment() {
        LockGuard lock(mMutex);
        return ++mValue;
    }
    
    int32_t decrement() {
        LockGuard lock(mMutex);
        return --mValue;
    }
    
    int32_t get() const {
        LockGuard lock(mMutex);
        return mValue;
    }
    
    void set(int32_t value) {
        LockGuard lock(mMutex);
        mValue = value;
    }
};

// Thread-safe flag
class ThreadSafeFlag {
private:
    bool mFlag;
    mutable Mutex mMutex;
    
public:
    ThreadSafeFlag(bool initialValue = false) : mFlag(initialValue) {}
    
    void set(bool value) {
        LockGuard lock(mMutex);
        mFlag = value;
    }
    
    bool get() const {
        LockGuard lock(mMutex);
        return mFlag;
    }
    
    bool compareAndSet(bool expected, bool desired) {
        LockGuard lock(mMutex);
        if (mFlag == expected) {
            mFlag = desired;
            return true;
        }
        return false;
    }
};

// Cleanup function for all threads
void cleanupAllThreads() {
    // Destroy thread pool
    destroyThreadPool();
}

} // namespace platform_threading

#endif // PLATFORM_WIIU
