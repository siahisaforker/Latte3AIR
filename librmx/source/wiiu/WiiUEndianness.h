#pragma once

#include <cstdint>
#include <cstring>

namespace rmx {

class WiiUEndianness
{
public:
    // Endianness conversion functions
    static uint16_t swap16(uint16_t value);
    static uint32_t swap32(uint32_t value);
    static uint64_t swap64(uint64_t value);
    
    static int16_t swap16(int16_t value);
    static int32_t swap32(int32_t value);
    static int64_t swap64(int64_t value);
    
    // Safe memory access functions (handle unaligned reads/writes)
    static uint16_t readUnaligned16(const void* ptr);
    static uint32_t readUnaligned32(const void* ptr);
    static uint64_t readUnaligned64(const void* ptr);
    
    static void writeUnaligned16(void* ptr, uint16_t value);
    static void writeUnaligned32(void* ptr, uint32_t value);
    static void writeUnaligned64(void* ptr, uint64_t value);
    
    // ROM data handling (ROMs are typically little-endian)
    static uint16_t readROM16(const void* ptr);
    static uint32_t readROM32(const void* ptr);
    static uint64_t readROM64(const void* ptr);
    
    // Alignment utilities
    static constexpr size_t alignUp(size_t size, size_t alignment);
    static constexpr bool isAligned(size_t value, size_t alignment);
    static void* alignedAlloc(size_t size, size_t alignment);
    static void alignedFree(void* ptr);
    
    // Memory pool for frequent allocations
    static void* poolAlloc(size_t size);
    static void poolFree(void* ptr);
    static void cleanupPool();

private:
    // Wii U is big-endian, so we need to swap little-endian data
    static constexpr bool kIsBigEndian = true;
    
    // Memory pool constants
    static constexpr size_t kPoolSize = 1024 * 1024; // 1MB pool
    static uint8_t* sMemoryPool;
    static size_t sPoolOffset;
    static bool sPoolInitialized;
};

// Inline implementations
inline uint16_t WiiUEndianness::swap16(uint16_t value)
{
    return ((value & 0xFF) << 8) | ((value >> 8) & 0xFF);
}

inline uint32_t WiiUEndianness::swap32(uint32_t value)
{
    return ((value & 0xFF) << 24) |
           ((value & 0xFF00) << 8) |
           ((value >> 8) & 0xFF00) |
           ((value >> 24) & 0xFF);
}

inline uint64_t WiiUEndianness::swap64(uint64_t value)
{
    return ((value & 0xFFULL) << 56) |
           ((value & 0xFF00ULL) << 40) |
           ((value & 0xFF0000ULL) << 24) |
           ((value & 0xFF000000ULL) << 8) |
           ((value >> 8) & 0xFF000000ULL) |
           ((value >> 24) & 0xFF0000ULL) |
           ((value >> 40) & 0xFF00ULL) |
           ((value >> 56) & 0xFFULL);
}

inline int16_t WiiUEndianness::swap16(int16_t value)
{
    return static_cast<int16_t>(swap16(static_cast<uint16_t>(value)));
}

inline int32_t WiiUEndianness::swap32(int32_t value)
{
    return static_cast<int32_t>(swap32(static_cast<uint32_t>(value)));
}

inline int64_t WiiUEndianness::swap64(int64_t value)
{
    return static_cast<int64_t>(swap64(static_cast<uint64_t>(value)));
}

inline uint16_t WiiUEndianness::readUnaligned16(const void* ptr)
{
    uint16_t value;
    std::memcpy(&value, ptr, sizeof(value));
    return value;
}

inline uint32_t WiiUEndianness::readUnaligned32(const void* ptr)
{
    uint32_t value;
    std::memcpy(&value, ptr, sizeof(value));
    return value;
}

inline uint64_t WiiUEndianness::readUnaligned64(const void* ptr)
{
    uint64_t value;
    std::memcpy(&value, ptr, sizeof(value));
    return value;
}

inline void WiiUEndianness::writeUnaligned16(void* ptr, uint16_t value)
{
    std::memcpy(ptr, &value, sizeof(value));
}

inline void WiiUEndianness::writeUnaligned32(void* ptr, uint32_t value)
{
    std::memcpy(ptr, &value, sizeof(value));
}

inline void WiiUEndianness::writeUnaligned64(void* ptr, uint64_t value)
{
    std::memcpy(ptr, &value, sizeof(value));
}

inline uint16_t WiiUEndianness::readROM16(const void* ptr)
{
    return swap16(readUnaligned16(ptr));
}

inline uint32_t WiiUEndianness::readROM32(const void* ptr)
{
    return swap32(readUnaligned32(ptr));
}

inline uint64_t WiiUEndianness::readROM64(const void* ptr)
{
    return swap64(readUnaligned64(ptr));
}

inline constexpr size_t WiiUEndianness::alignUp(size_t size, size_t alignment)
{
    return (size + alignment - 1) & ~(alignment - 1);
}

inline constexpr bool WiiUEndianness::isAligned(size_t value, size_t alignment)
{
    return (value & (alignment - 1)) == 0;
}

} // namespace rmx
