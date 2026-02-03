#pragma once

#include <cstdint>
#include <coreinit/thread.h>
#include <coreinit/mutex.h>
#include <coreinit/condition.h>

namespace wiiu {

typedef void (*ThreadFunction)(void*);
typedef OSThread* ThreadHandle;
typedef OSMutex* MutexHandle;
typedef OSCondition* ConditionHandle;

// Time functions
uint64_t getCurrentTime();
double getTimeInSeconds();
void sleep(uint32_t milliseconds);

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

}
