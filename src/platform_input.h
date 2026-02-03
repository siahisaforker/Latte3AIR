#pragma once

#if defined(PLATFORM_WIIU) || defined(__WIIU__)

#include <cstdint>
#include "platform_sdl.h"

namespace platform_input {

// Input initialization
bool initializeInput();
void shutdownInput();

// Input polling
void pollInput();

// Get current input state
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
    bool l = false;
    bool r = false;
    bool zl = false;
    bool zr = false;
    float leftStickX = 0.0f;
    float leftStickY = 0.0f;
    float rightStickX = 0.0f;
    float rightStickY = 0.0f;
    float leftTrigger = 0.0f;
    float rightTrigger = 0.0f;
};

const InputState& getInputState();

// Joystick functions (SDL compatibility)
int numJoysticks();
SDL_Joystick* joystickOpen(int device_index);
void joystickClose(SDL_Joystick* joystick);
SDL_JoystickID joystickInstanceID(SDL_Joystick* joystick);
int joystickNumButtons(SDL_Joystick* joystick);
Uint8 joystickGetButton(SDL_Joystick* joystick, int button);
int joystickNumAxes(SDL_Joystick* joystick);
Sint16 joystickGetAxis(SDL_Joystick* joystick, int axis);

// Event handling
int pollEvent(SDL_Event* event);

// Analog stick values
float getLeftStickX();
float getLeftStickY();
float getRightStickX();
float getRightStickY();

} // namespace platform_input

#endif // PLATFORM_WIIU
