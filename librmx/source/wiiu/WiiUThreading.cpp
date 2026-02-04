#include "WiiUThreading.h"
#include <coreinit/thread.h>
#include <coreinit/mutex.h>
#include <coreinit/condition.h>
#include <coreinit/time.h>
#include <memory>
#include <cstring>
#include <unordered_map>

namespace rmx {

static std::unordered_map<OSThread*, WiiUThread*> gThreadMap;

// wrapper entry that matches OSThreadEntryPointFn
static int WiiUThread_wrapper_entry(int argc, const char** argv)
{
    OSThread* cur = OSGetCurrentThread();
    auto it = gThreadMap.find(cur);
    if (it != gThreadMap.end())
    {
        WiiUThread* thread = it->second;
        if (thread)
        {
            thread->runThreadFunction();
        }
        gThreadMap.erase(it);
    }
    OSExitThread(0);
    return 0;
}

// WiiUMutex implementation
WiiUMutex::WiiUMutex() : mMutex(nullptr)
{
    mMutex = new OSMutex();
    std::memset(mMutex, 0, sizeof(OSMutex));
    OSInitMutex(mMutex);
}

WiiUMutex::~WiiUMutex()
{
    if (mMutex)
    {
        delete mMutex;
        mMutex = nullptr;
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
        return OSTryLockMutex(mMutex) != 0;
    }
    return false;
}

// WiiUConditionVariable implementation
WiiUConditionVariable::WiiUConditionVariable() : mCondition(nullptr)
{
    mCondition = new OSCondition();
    std::memset(mCondition, 0, sizeof(OSCondition));
    OSInitCond(mCondition);
}

WiiUConditionVariable::~WiiUConditionVariable()
{
    if (mCondition)
    {
        delete mCondition;
        mCondition = nullptr;
    }
}

void WiiUConditionVariable::signal()
{
    if (mCondition)
    {
        OSSignalCond(mCondition);
    }
}

void WiiUConditionVariable::wait(WiiUMutex& mutex)
{
    if (mCondition && mutex.getNativeMutex())
    {
        OSWaitCond(mCondition, mutex.getNativeMutex());
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

void WiiUThread::startInternal()
{
    if (mThread)
        return; // Already started

    mThread = new OSThread();
    std::memset(mThread, 0, sizeof(OSThread));

    BOOL ok = OSCreateThread(mThread,
                             &WiiUThread_wrapper_entry,
                             0,
                             nullptr,
                             nullptr,
                             static_cast<uint32_t>(kDefaultStackSize),
                             kDefaultPriority,
                             0);

    if (ok)
    {
        // record mapping from OSThread* to this wrapper
        gThreadMap[mThread] = this;
        mJoinable = true;
        OSResumeThread(mThread);
    }
    else
    {
        delete mThread;
        mThread = nullptr;
    }
}

void WiiUThread::join()
{
    if (mThread && mJoinable)
    {
        int result = 0;
        OSJoinThread(mThread, &result);
        // thread has exited; clean up
        delete mThread;
        mThread = nullptr;
        mJoinable = false;
    }
}

void WiiUThread::detach()
{
    if (mThread)
    {
        OSDetachThread(mThread);
        mJoinable = false;
    }
}

bool WiiUThread::joinable() const
{
    return mJoinable;
}

void WiiUThread::yield()
{
    OSYieldThread();
}

void WiiUThread::sleep(uint32_t milliseconds)
{
    OSSleepTicks(OSMillisecondsToTicks(milliseconds));
}

void WiiUThread::runThreadFunction()
{
    if (mFunction)
        mFunction();
}

} // namespace rmx
