#pragma once

#include <cstdint>
#include <cstring>
#include <cmath>
#include <cstdio>
#include "endianness.h"

namespace wiiu {

// Stack alignment macros for Wii U
#define ALIGN_STACK_16 __attribute__((aligned(16)))
#define ALIGN_STACK_32 __attribute__((aligned(32)))
#define ALIGN_STACK_64 __attribute__((aligned(64)))

// Safe struct alignment for GPU buffers
template<typename T>
struct AlignedStruct {
    alignas(16) T data;
    
    AlignedStruct() { memset(&data, 0, sizeof(T)); }
    AlignedStruct(const T& val) : data(val) {}
    
    T& operator*() { return data; }
    T* operator->() { return &data; }
    const T& operator*() const { return data; }
    const T* operator->() const { return &data; }
};

// Safe array wrapper with bounds checking
template<typename T, size_t Size>
class SafeArray {
private:
    alignas(16) T m_data[Size];
    
public:
    SafeArray() { memset(m_data, 0, sizeof(m_data)); }
    
    T& operator[](size_t index) {
        if (index >= Size) {
            // Return first element as fallback to prevent crash
            return m_data[0];
        }
        return m_data[index];
    }
    
    const T& operator[](size_t index) const {
        if (index >= Size) {
            return m_data[0];
        }
        return m_data[index];
    }
    
    T* data() { return m_data; }
    const T* data() const { return m_data; }
    size_t size() const { return Size; }
    
    void clear() { memset(m_data, 0, sizeof(m_data)); }
};

// Safe pointer wrapper
template<typename T>
class SafePtr {
private:
    T* m_ptr;
    
public:
    SafePtr() : m_ptr(nullptr) {}
    SafePtr(T* ptr) : m_ptr(ptr) {}
    SafePtr(std::nullptr_t) : m_ptr(nullptr) {}
    
    T& operator*() {
        if (!m_ptr) {
            static T dummy;
            return dummy;
        }
        return *m_ptr;
    }
    
    T* operator->() {
        return m_ptr ? m_ptr : &getDummy();
    }
    
    T* get() const { return m_ptr; }
    void reset(T* ptr = nullptr) { m_ptr = ptr; }
    bool isNull() const { return !m_ptr; }
    
private:
    static T& getDummy() {
        static T dummy;
        return dummy;
    }
};

// Delta time clamping to prevent physics explosions
class DeltaTimeClamper {
private:
    static constexpr float MIN_DELTA = 0.001f;  // 1ms
    static constexpr float MAX_DELTA = 0.1f;    // 100ms
    static constexpr float DEFAULT_DELTA = 1.0f / 60.0f; // 60 FPS
    
public:
    static float clamp(float deltaTime) {
        if (std::isnan(deltaTime) || std::isinf(deltaTime) || deltaTime <= 0.0f) {
            return DEFAULT_DELTA;
        }
        
        if (deltaTime < MIN_DELTA) return MIN_DELTA;
        if (deltaTime > MAX_DELTA) return MAX_DELTA;
        return deltaTime;
    }
};

// Memory safety utilities
class MemorySafety {
public:
    static void safeMemcpy(void* dest, const void* src, size_t size) {
        if (!dest || !src || size == 0) return;
        memcpy(dest, src, size);
    }
    
    static void safeMemset(void* ptr, int value, size_t size) {
        if (!ptr || size == 0) return;
        memset(ptr, value, size);
    }
    
    static bool isValidPointer(const void* ptr) {
        return ptr != nullptr;
    }
};

// File I/O safety
class FileIOSafety {
public:
    static bool safeRead(FILE* file, void* buffer, size_t size) {
        if (!file || !buffer || size == 0) return false;
        return fread(buffer, 1, size, file) == size;
    }
    
    static bool safeWrite(FILE* file, const void* buffer, size_t size) {
        if (!file || !buffer || size == 0) return false;
        return fwrite(buffer, 1, size, file) == size;
    }
    
    static bool safeSeek(FILE* file, long offset, int whence) {
        if (!file) return false;
        return fseek(file, offset, whence) == 0;
    }
};

// Initialization safety
class InitSafety {
public:
    template<typename T>
    static bool safeInit(T& obj) {
        try {
            obj = T(); // Default construct
            return true;
        } catch (...) {
            return false;
        }
    }
    
    template<typename T>
    static void safeDelete(T*& ptr) {
        if (ptr) {
            delete ptr;
            ptr = nullptr;
        }
    }
    
    template<typename T>
    static void safeDeleteArray(T*& ptr) {
        if (ptr) {
            delete[] ptr;
            ptr = nullptr;
        }
    }
};

// Math safety
class MathSafety {
public:
    static float safeDivide(float numerator, float denominator, float fallback = 0.0f) {
        if (std::abs(denominator) < 1e-6f) return fallback;
        return numerator / denominator;
    }
    
    static float safeSqrt(float value, float fallback = 0.0f) {
        if (value < 0.0f || std::isnan(value) || std::isinf(value)) return fallback;
        return std::sqrt(value);
    }
    
    static float safeAtan2(float y, float x, float fallback = 0.0f) {
        if (std::isnan(y) || std::isnan(x) || std::isinf(y) || std::isinf(x)) return fallback;
        return std::atan2(y, x);
    }
};

}
