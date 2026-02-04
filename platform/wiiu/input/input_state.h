#pragma once
#include <cstdint>

struct InputState {
    uint32_t _placeholder = 0;
};

inline void pollWiiUInput(InputState& state)
{
    (void)state; // stub: no input on host
}

inline uint16_t inputStateToOxygenFlags(const InputState& state)
{
    (void)state;
    return 0;
}
