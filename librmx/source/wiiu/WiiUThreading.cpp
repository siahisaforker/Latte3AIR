#include "WiiUThreading.h"
#include <coreinit/thread.h>
#include <coreinit/mutex.h>
#include <coreinit/condition.h>
#include <coreinit/time.h>
#include <memory>
#include <cstring>
#include <unordered_map>
#include <malloc.h>

namespace rmx {

static OSMutex gThreadMapMutex;
static bool gThreadMapMutexInit = false;
static std::unordered_map<OSThread*, WiiUThread*> gThreadMap;

static void ensureMapMutex()
{
    if (!gThreadMapMutexInit)
    {
        std::memset(&gThreadMapMutex, 0, sizeof(OSMutex));
        OSInitMutex(&gThreadMapMutex);
        gThreadMapMutexInit = true;
    }
}

// wrapper entry that matches OSThreadEntryPointFn
static int WiiUThread_wrapper_entry(int argc, const char** argv)
{
    OSThread* cur = OSGetCurrentThread();

    ensureMapMutex();
    OSLockMutex(&gThreadMapMutex);
    auto it = gThreadMap.find(cur);
    WiiUThread* thread = (it != gThreadMap.end()) ? it->second : nullptr;
    if (it != gThreadMap.end())
        gThreadMap.erase(it);
    OSUnlockMutex(&gThreadMapMutex);

    if (thread)
        thread->runThreadFunction();

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
WiiUThread::WiiUThread() : mThread(nullptr), mStack(nullptr), mJoinable(false)
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

    // Allocate the thread stack (PPC stacks grow downward)
    mStack = memalign(8, kDefaultStackSize);
    if (!mStack)
        return;

    mThread = new OSThread();
    std::memset(mThread, 0, sizeof(OSThread));

    // OSCreateThread expects the stack TOP (base + size) because PPC stacks grow down
    void* stackTop = static_cast<uint8_t*>(mStack) + kDefaultStackSize;

    BOOL ok = OSCreateThread(mThread,
                             &WiiUThread_wrapper_entry,
                             0,
                             nullptr,
                             stackTop,
                             static_cast<uint32_t>(kDefaultStackSize),
                             kDefaultPriority,
                             OS_THREAD_ATTRIB_AFFINITY_ANY);

    if (ok)
    {
        ensureMapMutex();
        OSLockMutex(&gThreadMapMutex);
        gThreadMap[mThread] = this;
        OSUnlockMutex(&gThreadMapMutex);
        mJoinable = true;
        OSResumeThread(mThread);
    }
    else
    {
        delete mThread;
        mThread = nullptr;
        free(mStack);
        mStack = nullptr;
    }
}

void WiiUThread::join()
{
    if (mThread && mJoinable)
    {
        int result = 0;
        OSJoinThread(mThread, &result);
        delete mThread;
        mThread = nullptr;
        if (mStack)
        {
            free(mStack);
            mStack = nullptr;
        }
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
