#pragma once

#include <cstdint>

namespace wiiu {

struct InputState {
    bool up = false;
    bool down = false;
    bool left = false;
    bool right = false;
    bool a = false;
    bool b = false;
    bool x = false;
    bool y = false;
    bool start = false;
    bool select = false;
};

void initializeInput();
void pollInput(InputState& state);
uint16_t inputStateToOxygenFlags(const InputState& state);

}
