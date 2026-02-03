#include "threading.h"
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

// Thread wrapper
struct ThreadData {
    ThreadFunction func;
    void* arg;
};

static void threadEntry(int32_t arg) {
    ThreadData* data = (ThreadData*)arg;
    data->func(data->arg);
    delete data;
}

ThreadHandle createThread(ThreadFunction func, void* arg, int priority, uint32_t stackSize) {
    ThreadData* data = new ThreadData{func, arg};
    
    OSThread* thread = new OSThread;
    bool created = OSCreateThread(
        thread,
        (OSThreadEntryPointFn)threadEntry,
        (int32_t)data,
        (char*)stackSize, // Stack top (cast from size)
        nullptr, // Stack base (auto-allocated)
        stackSize,
        priority,
        OS_THREAD_ATTRIB_AFFINITY_ANY
    );
    
    if (!created) {
        delete data;
        delete thread;
        return nullptr;
    }
    
    return thread;
}

void startThread(ThreadHandle thread) {
    OSResumeThread(thread);
}

void joinThread(ThreadHandle thread) {
    int32_t result;
    OSJoinThread(thread, &result);
    delete thread;
}

void destroyThread(ThreadHandle thread) {
    OSDetachThread(thread);
    delete thread;
}

// Mutex
MutexHandle createMutex() {
    OSMutex* mutex = new OSMutex;
    OSInitMutex(mutex);
    return mutex;
}

void lockMutex(MutexHandle mutex) {
    OSLockMutex(mutex);
}

void unlockMutex(MutexHandle mutex) {
    OSUnlockMutex(mutex);
}

void destroyMutex(MutexHandle mutex) {
    delete mutex;
}

// Condition variable
ConditionHandle createCondition() {
    OSCondition* cond = new OSCondition;
    OSInitCond(cond);
    return cond;
}

void waitCondition(ConditionHandle cond, MutexHandle mutex) {
    OSWaitCond(cond, mutex);
}

void signalCondition(ConditionHandle cond) {
    OSSignalCond(cond);
}

void broadcastCondition(ConditionHandle cond) {
    OSSignalCond(cond); // Wii U doesn't have broadcast, use signal
}

void destroyCondition(ConditionHandle cond) {
    delete cond;
}

}
