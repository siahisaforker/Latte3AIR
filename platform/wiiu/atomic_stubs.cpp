extern "C" {
    unsigned long long __atomic_fetch_add_8(volatile void* ptr, unsigned long long val, int memorder)
    {
        volatile unsigned long long* p = (volatile unsigned long long*)ptr;
        unsigned long long old = *p;
        *p = old + val;
        return old;
    }

    unsigned long long __atomic_load_8(const volatile void* ptr, int memorder)
    {
        const volatile unsigned long long* p = (const volatile unsigned long long*)ptr;
        return *p;
    }
}
