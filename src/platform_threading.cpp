#include <os.h>
#include <cstdio>

typedef void (*ThreadFunc)(void*);

struct Thread {
    OSThread osThread;
};

bool create_thread(Thread& thread, ThreadFunc func, void* arg, size_t stackSize = 0x4000) {
    return OSCreateThread(&thread.osThread, (OSThreadFunction)func, arg, nullptr, stackSize, 30, OS_THREAD_ATTR_DETACHED) == 0;
}

void sleep_ms(u32 ms) {
    OSSleepTicks(ms * (OSGetTickFrequency() / 1000));
}

u64 get_time_ms() {
    return OSGetSystemTick() / (OSGetTickFrequency() / 1000);
}
