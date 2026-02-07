#include <coreinit/interrupts.h>

extern "C" {
    unsigned long long __atomic_fetch_add_8(volatile void* ptr, unsigned long long val, int /*memorder*/)
    {
        BOOL enabled = OSDisableInterrupts();
        volatile unsigned long long* p = (volatile unsigned long long*)ptr;
        unsigned long long old = *p;
        *p = old + val;
        OSRestoreInterrupts(enabled);
        return old;
    }

    unsigned long long __atomic_load_8(const volatile void* ptr, int /*memorder*/)
    {
        BOOL enabled = OSDisableInterrupts();
        const volatile unsigned long long* p = (const volatile unsigned long long*)ptr;
        unsigned long long result = *p;
        OSRestoreInterrupts(enabled);
        return result;
    }

    void __atomic_store_8(volatile void* ptr, unsigned long long val, int /*memorder*/)
    {
        BOOL enabled = OSDisableInterrupts();
        volatile unsigned long long* p = (volatile unsigned long long*)ptr;
        *p = val;
        OSRestoreInterrupts(enabled);
    }

    unsigned long long __atomic_exchange_8(volatile void* ptr, unsigned long long val, int /*memorder*/)
    {
        BOOL enabled = OSDisableInterrupts();
        volatile unsigned long long* p = (volatile unsigned long long*)ptr;
        unsigned long long old = *p;
        *p = val;
        OSRestoreInterrupts(enabled);
        return old;
    }

    bool __atomic_compare_exchange_8(volatile void* ptr, void* expected, unsigned long long desired,
                                    bool /*weak*/, int /*success_memorder*/, int /*failure_memorder*/)
    {
        BOOL enabled = OSDisableInterrupts();
        volatile unsigned long long* p = (volatile unsigned long long*)ptr;
        unsigned long long* exp = (unsigned long long*)expected;
        if (*p == *exp)
        {
            *p = desired;
            OSRestoreInterrupts(enabled);
            return 1;
        }
        *exp = *p;
        OSRestoreInterrupts(enabled);
        return 0;
    }

    unsigned long long __atomic_fetch_sub_8(volatile void* ptr, unsigned long long val, int /*memorder*/)
    {
        BOOL enabled = OSDisableInterrupts();
        volatile unsigned long long* p = (volatile unsigned long long*)ptr;
        unsigned long long old = *p;
        *p = old - val;
        OSRestoreInterrupts(enabled);
        return old;
    }
}
