#pragma once

#include <cstdint>
#include <cstring>
#include <cmath>

namespace wiiu {

// Big-endian read functions (Wii U is big-endian, ROMs are little-endian)
inline uint16_t read16le(const void* ptr) {
    const uint8_t* b = (const uint8_t*)ptr;
    return (b[1] << 8) | b[0];
}

inline uint32_t read32le(const void* ptr) {
    const uint8_t* b = (const uint8_t*)ptr;
    return (b[3] << 24) | (b[2] << 16) | (b[1] << 8) | b[0];
}

inline uint64_t read64le(const void* ptr) {
    const uint8_t* b = (const uint8_t*)ptr;
    return ((uint64_t)b[7] << 56) | ((uint64_t)b[6] << 48) | ((uint64_t)b[5] << 40) | ((uint64_t)b[4] << 32) |
           ((uint64_t)b[3] << 24) | ((uint64_t)b[2] << 16) | ((uint64_t)b[1] << 8) | b[0];
}

// Little-endian write functions
inline void write16le(void* ptr, uint16_t value) {
    uint8_t* b = (uint8_t*)ptr;
    b[0] = value & 0xFF;
    b[1] = (value >> 8) & 0xFF;
}

inline void write32le(void* ptr, uint32_t value) {
    uint8_t* b = (uint8_t*)ptr;
    b[0] = value & 0xFF;
    b[1] = (value >> 8) & 0xFF;
    b[2] = (value >> 16) & 0xFF;
    b[3] = (value >> 24) & 0xFF;
}

inline void write64le(void* ptr, uint64_t value) {
    uint8_t* b = (uint8_t*)ptr;
    b[0] = value & 0xFF;
    b[1] = (value >> 8) & 0xFF;
    b[2] = (value >> 16) & 0xFF;
    b[3] = (value >> 24) & 0xFF;
    b[4] = (value >> 32) & 0xFF;
    b[5] = (value >> 40) & 0xFF;
    b[6] = (value >> 48) & 0xFF;
    b[7] = (value >> 56) & 0xFF;
}

// Big-endian read functions (for native Wii U data)
inline uint16_t read16be(const void* ptr) {
    const uint8_t* b = (const uint8_t*)ptr;
    return (b[0] << 8) | b[1];
}

inline uint32_t read32be(const void* ptr) {
    const uint8_t* b = (const uint8_t*)ptr;
    return (b[0] << 24) | (b[1] << 16) | (b[2] << 8) | b[3];
}

inline uint64_t read64be(const void* ptr) {
    const uint8_t* b = (const uint8_t*)ptr;
    return ((uint64_t)b[0] << 56) | ((uint64_t)b[1] << 48) | ((uint64_t)b[2] << 40) | ((uint64_t)b[3] << 32) |
           ((uint64_t)b[4] << 24) | ((uint64_t)b[5] << 16) | ((uint64_t)b[6] << 8) | b[7];
}

// Big-endian write functions
inline void write16be(void* ptr, uint16_t value) {
    uint8_t* b = (uint8_t*)ptr;
    b[0] = (value >> 8) & 0xFF;
    b[1] = value & 0xFF;
}

inline void write32be(void* ptr, uint32_t value) {
    uint8_t* b = (uint8_t*)ptr;
    b[0] = (value >> 24) & 0xFF;
    b[1] = (value >> 16) & 0xFF;
    b[2] = (value >> 8) & 0xFF;
    b[3] = value & 0xFF;
}

inline void write64be(void* ptr, uint64_t value) {
    uint8_t* b = (uint8_t*)ptr;
    b[0] = (value >> 56) & 0xFF;
    b[1] = (value >> 48) & 0xFF;
    b[2] = (value >> 40) & 0xFF;
    b[3] = (value >> 32) & 0xFF;
    b[4] = (value >> 24) & 0xFF;
    b[5] = (value >> 16) & 0xFF;
    b[6] = (value >> 8) & 0xFF;
    b[7] = value & 0xFF;
}

// Safe memory copy with endianness conversion
template<typename T>
inline void readAndConvert(const void* src, T& dest, bool littleEndian = true) {
    if (littleEndian) {
        if constexpr (sizeof(T) == 2) {
            dest = read16le(src);
        } else if constexpr (sizeof(T) == 4) {
            dest = read32le(src);
        } else if constexpr (sizeof(T) == 8) {
            dest = read64le(src);
        } else {
            memcpy(&dest, src, sizeof(T)); // Fallback for other sizes
        }
    } else {
        if constexpr (sizeof(T) == 2) {
            dest = read16be(src);
        } else if constexpr (sizeof(T) == 4) {
            dest = read32be(src);
        } else if constexpr (sizeof(T) == 8) {
            dest = read64be(src);
        } else {
            memcpy(&dest, src, sizeof(T)); // Fallback for other sizes
        }
    }
}

// Float safety utilities
inline bool isValidFloat(float value) {
    return !std::isnan(value) && !std::isinf(value);
}

inline float clampFloat(float value, float min, float max) {
    if (std::isnan(value)) return min;
    if (std::isinf(value)) return (value > 0) ? max : min;
    return (value < min) ? min : (value > max) ? max : value;
}

}
