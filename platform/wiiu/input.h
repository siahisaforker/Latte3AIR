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
    bool l = false;      // Left trigger
    bool r = false;      // Right trigger
    bool zl = false;     // ZL button
    bool zr = false;     // ZR button
    float leftStickX = 0.0f;
    float leftStickY = 0.0f;
    float rightStickX = 0.0f;
    float rightStickY = 0.0f;
    float leftTrigger = 0.0f;
    float rightTrigger = 0.0f;
};

void initializeInput();
void pollInput(InputState& state);
uint16_t inputStateToOxygenFlags(const InputState& state);

}
