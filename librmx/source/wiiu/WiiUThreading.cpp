#include "WiiUThreading.h"
#include <coreinit/thread.h>
#include <coreinit/mutex.h>
#include <coreinit/condition.h>
#include <coreinit/time.h>
#include <memory>
#include <cstring>

namespace rmx {

// WiiUMutex implementation
WiiUMutex::WiiUMutex() : mMutex(nullptr)
{
    mMutex = OSCreateMutex();
}

WiiUMutex::~WiiUMutex()
{
    if (mMutex)
    {
        OSDestroyMutex(mMutex);
    }
}

void WiiUMutex::lock()
{
    if (mMutex)
    {
        OSLockMutex(mMutex);
    }
}

void WiiUMutex::unlock()
{
    if (mMutex)
    {
        OSUnlockMutex(mMutex);
    }
}

bool WiiUMutex::tryLock()
{
    if (mMutex)
    {
        return OSTryLockMutex(mMutex);
    }
    return false;
}

// WiiUConditionVariable implementation
WiiUConditionVariable::WiiUConditionVariable() : mCondition(nullptr)
{
    mCondition = OSCreateCondition();
}

WiiUConditionVariable::~WiiUConditionVariable()
{
    if (mCondition)
    {
        OSDestroyCondition(mCondition);
    }
}

void WiiUConditionVariable::signal()
{
    if (mCondition)
    {
        OSSignalCondition(mCondition);
    }
}

void WiiUConditionVariable::wait(WiiUMutex& mutex)
{
    if (mCondition && mutex.getNativeMutex())
    {
        OSWaitCondition(mCondition, mutex.getNativeMutex());
    }
}

// WiiUThread implementation
WiiUThread::WiiUThread() : mThread(nullptr), mJoinable(false)
{
}

WiiUThread::~WiiUThread()
{
    if (joinable())
    {
        join();
    }
}

void WiiUThread::threadEntryPoint(void* arg)
{
    WiiUThread* thread = static_cast<WiiUThread*>(arg);
    if (thread && thread->mFunction)
    {
        thread->mFunction();
    }
}

void WiiUThread::startInternal()
{
    if (mThread)
    {
        return; // Already started
    }
    
    mThread = OSCreateThread(
        threadEntryPoint,
        this,
        kDefaultStackSize,
        kDefaultPriority,
        nullptr
    );
    
    if (mThread)
    {
        mJoinable = true;
        OSResumeThread(mThread);
    }
}

void WiiUThread::join()
{
    if (mThread && mJoinable)
    {
        OSJoinThread(mThread, nullptr);
        OSDestroyThread(mThread);
        mThread = nullptr;
        mJoinable = false;
    }
}

void WiiUThread::detach()
{
    if (mThread)
    {
        // Wii U doesn't have a direct detach equivalent
        // We'll just mark it as non-joinable
        mJoinable = false;
    }
}

bool WiiUThread::joinable() const
{
    return mJoinable;
}

void WiiUThread::yield()
{
    OSThreadYield();
}

void WiiUThread::sleep(uint32_t milliseconds)
{
    OSSleep(milliseconds * 1000000ULL); // Convert to nanoseconds
}

} // namespace rmx
