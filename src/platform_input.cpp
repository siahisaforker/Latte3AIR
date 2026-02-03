#include "platform_input.h"

#if defined(PLATFORM_WIIU) || defined(__WIIU__)

#include "../platform/wiiu/input.h"
#include <cstring>
#include <algorithm>
#include <cmath>

// Complete input implementation using Wii U VPAD/KPAD
namespace platform_input {

static bool sInputInitialized = false;
static platform_input::InputState sCurrentState;
static platform_input::InputState sPreviousState;

// Deadzone configuration
static constexpr float ANALOG_DEADZONE = 0.15f;
static constexpr float ANALOG_MAX = 1.0f;
static constexpr float ANALOG_MIN = -1.0f;

// Button mapping for Sonic 3 controls
enum SonicButton {
    SONIC_BUTTON_UP = 0x01,
    SONIC_BUTTON_DOWN = 0x02,
    SONIC_BUTTON_LEFT = 0x04,
    SONIC_BUTTON_RIGHT = 0x08,
    SONIC_BUTTON_A = 0x10,      // Jump
    SONIC_BUTTON_B = 0x20,      // Spin Dash
    SONIC_BUTTON_C = 0x40,      // (unused)
    SONIC_BUTTON_X = 0x80,      // (unused)
    SONIC_BUTTON_Y = 0x100,     // (unused)
    SONIC_BUTTON_Z = 0x200,     // (unused)
    SONIC_BUTTON_START = 0x400,
    SONIC_BUTTON_SELECT = 0x800
};

bool initializeInput() {
    if (sInputInitialized) return true;
    
    wiiu::initializeInput();
    sInputInitialized = true;
    
    // Clear initial state
    memset(&sCurrentState, 0, sizeof(sCurrentState));
    memset(&sPreviousState, 0, sizeof(sPreviousState));
    
    return true;
}

void shutdownInput() {
    if (!sInputInitialized) return;
    
    sInputInitialized = false;
}

void pollInput() {
    if (!sInputInitialized) return;
    
    // Store previous state for change detection
    sPreviousState = sCurrentState;
    
    // Poll Wii U input and convert
    wiiu::InputState wiiuState;
    wiiu::pollInput(wiiuState);
    
    // Convert wiiu::InputState to platform_input::InputState
    sCurrentState.up = wiiuState.up;
    sCurrentState.down = wiiuState.down;
    sCurrentState.left = wiiuState.left;
    sCurrentState.right = wiiuState.right;
    sCurrentState.a = wiiuState.a;
    sCurrentState.b = wiiuState.b;
    sCurrentState.x = wiiuState.x;
    sCurrentState.y = wiiuState.y;
    sCurrentState.start = wiiuState.start;
    sCurrentState.select = wiiuState.select;
    sCurrentState.l = wiiuState.l;
    sCurrentState.r = wiiuState.r;
    sCurrentState.zl = wiiuState.zl;
    sCurrentState.zr = wiiuState.zr;
    sCurrentState.leftStickX = wiiuState.leftStickX;
    sCurrentState.leftStickY = wiiuState.leftStickY;
    sCurrentState.rightStickX = wiiuState.rightStickX;
    sCurrentState.rightStickY = wiiuState.rightStickY;
    sCurrentState.leftTrigger = wiiuState.leftTrigger;
    sCurrentState.rightTrigger = wiiuState.rightTrigger;
}

// Get current input state
const platform_input::InputState& getInputState() {
    return sCurrentState;
}

// Joystick functions (SDL compatibility)
int numJoysticks() {
    return sInputInitialized ? 1 : 0; // Wii U has one GamePad
}

SDL_Joystick* joystickOpen(int device_index) {
    if (!sInputInitialized || device_index != 0) return nullptr;
    
    // Return a fake joystick handle for the GamePad
    return reinterpret_cast<SDL_Joystick*>(1);
}

void joystickClose(SDL_Joystick* joystick) {
    (void)joystick;
    // No cleanup needed for fake handle
}

SDL_JoystickID joystickInstanceID(SDL_Joystick* joystick) {
    (void)joystick;
    return 0; // Always return ID 0 for GamePad
}

int joystickNumButtons(SDL_Joystick* joystick) {
    if (!sInputInitialized) return 0;
    
    // Count active buttons
    int count = 0;
    if (sCurrentState.a) count++;
    if (sCurrentState.b) count++;
    if (sCurrentState.x) count++;
    if (sCurrentState.y) count++;
    if (sCurrentState.start) count++;
    if (sCurrentState.select) count++;
    if (sCurrentState.l) count++; // Left trigger
    if (sCurrentState.r) count++; // Right trigger
    if (sCurrentState.zl) count++; // ZL button
    if (sCurrentState.zr) count++; // ZR button
    
    return count;
}

Uint8 joystickGetButton(SDL_Joystick* joystick, int button) {
    if (!sInputInitialized) return 0;
    
    // Map button indices to actual buttons
    switch (button) {
        case 0: return sCurrentState.a ? 1 : 0;      // A
        case 1: return sCurrentState.b ? 1 : 0;      // B
        case 2: return sCurrentState.x ? 1 : 0;      // X
        case 3: return sCurrentState.y ? 1 : 0;      // Y
        case 4: return sCurrentState.start ? 1 : 0;  // Start/Plus
        case 5: return sCurrentState.select ? 1 : 0; // Select/Minus
        case 6: return sCurrentState.l ? 1 : 0;      // Left trigger
        case 7: return sCurrentState.r ? 1 : 0;      // Right trigger
        case 8: return sCurrentState.zl ? 1 : 0;     // ZL button
        case 9: return sCurrentState.zr ? 1 : 0;     // ZR button
        case 10: return sCurrentState.up ? 1 : 0;    // D-Pad Up
        case 11: return sCurrentState.down ? 1 : 0;  // D-Pad Down
        case 12: return sCurrentState.left ? 1 : 0;  // D-Pad Left
        case 13: return sCurrentState.right ? 1 : 0; // D-Pad Right
        default: return 0;
    }
}

int joystickNumAxes(SDL_Joystick* joystick) {
    if (!sInputInitialized) return 0;
    
    // Return 6 axes (Left X/Y, Right X/Y, Left/Right triggers)
    return 6;
}

Sint16 joystickGetAxis(SDL_Joystick* joystick, int axis) {
    if (!sInputInitialized) return 0;
    
    // Map axis indices with deadzone and scaling
    float value = 0.0f;
    
    switch (axis) {
        case 0: // Left stick X
            value = sCurrentState.leftStickX;
            break;
        case 1: // Left stick Y
            value = sCurrentState.leftStickY;
            break;
        case 2: // Right stick X
            value = sCurrentState.rightStickX;
            break;
        case 3: // Right stick Y
            value = sCurrentState.rightStickY;
            break;
        case 4: // Left trigger
            value = sCurrentState.leftTrigger;
            break;
        case 5: // Right trigger
            value = sCurrentState.rightTrigger;
            break;
        default:
            return 0;
    }
    
    // Apply deadzone
    if (std::abs(value) < ANALOG_DEADZONE) {
        value = 0.0f;
    }
    
    // Clamp to valid range and scale to Sint16
    value = std::max(ANALOG_MIN, std::min(ANALOG_MAX, value));
    return static_cast<Sint16>(value * 32767.0f);
}

// Event handling
int pollEvent(SDL_Event* event) {
    if (!sInputInitialized || !event) return 0;
    
    // Clear event
    memset(event, 0, sizeof(SDL_Event));
    
    // Poll input first
    pollInput();
    
    // Check for button changes
    for (int i = 0; i < joystickNumButtons(nullptr); ++i) {
        bool currentState = joystickGetButton(nullptr, i);
        bool previousState = false;
        
        // Map button to previous state
        switch (i) {
            case 0: previousState = sPreviousState.a; break;
            case 1: previousState = sPreviousState.b; break;
            case 2: previousState = sPreviousState.x; break;
            case 3: previousState = sPreviousState.y; break;
            case 4: previousState = sPreviousState.start; break;
            case 5: previousState = sPreviousState.select; break;
            case 6: previousState = sPreviousState.l; break;
            case 7: previousState = sPreviousState.r; break;
            case 8: previousState = sPreviousState.zl; break;
            case 9: previousState = sPreviousState.zr; break;
            case 10: previousState = sPreviousState.up; break;
            case 11: previousState = sPreviousState.down; break;
            case 12: previousState = sPreviousState.left; break;
            case 13: previousState = sPreviousState.right; break;
            default: previousState = false; break;
        }
        
        if (currentState && !previousState) {
            // Button pressed
            event->type = SDL_JOYBUTTONDOWN;
            event->data.button.type = SDL_JOYBUTTONDOWN;
            event->data.button.state = 1;
            event->data.button.which = 0;
            return 1;
        } else if (!currentState && previousState) {
            // Button released
            event->type = SDL_JOYBUTTONUP;
            event->data.button.type = SDL_JOYBUTTONUP;
            event->data.button.state = 0;
            event->data.button.which = 0;
            return 1;
        }
    }
    
    // Check for axis motion
    for (int i = 0; i < joystickNumAxes(nullptr); ++i) {
        Sint16 currentAxis = joystickGetAxis(nullptr, i);
        Sint16 previousAxis = 0;
        
        // Get previous axis value
        switch (i) {
            case 0: previousAxis = static_cast<Sint16>(sPreviousState.leftStickX * 32767.0f); break;
            case 1: previousAxis = static_cast<Sint16>(sPreviousState.leftStickY * 32767.0f); break;
            case 2: previousAxis = static_cast<Sint16>(sPreviousState.rightStickX * 32767.0f); break;
            case 3: previousAxis = static_cast<Sint16>(sPreviousState.rightStickY * 32767.0f); break;
            case 4: previousAxis = static_cast<Sint16>(sPreviousState.leftTrigger * 32767.0f); break;
            case 5: previousAxis = static_cast<Sint16>(sPreviousState.rightTrigger * 32767.0f); break;
            default: previousAxis = 0; break;
        }
        
        if (abs(currentAxis - previousAxis) > 100) { // Deadzone
            event->type = SDL_JOYAXISMOTION;
            event->data.axis.type = SDL_JOYAXISMOTION;
            event->data.axis.which = 0;
            event->data.axis.value = currentAxis;
            return 1;
        }
    }
    
    return 0;
}

// Analog stick values (for internal use)
float getLeftStickX() {
    return sCurrentState.leftStickX;
}

float getLeftStickY() {
    return sCurrentState.leftStickY;
}

float getRightStickX() {
    return sCurrentState.rightStickX;
}

float getRightStickY() {
    return sCurrentState.rightStickY;
}

// Trigger values
float getLeftTrigger() {
    return sCurrentState.leftTrigger;
}

float getRightTrigger() {
    return sCurrentState.rightTrigger;
}

// Button state helpers
bool isButtonPressed(SonicButton button) {
    switch (button) {
        case SONIC_BUTTON_UP: return sCurrentState.up;
        case SONIC_BUTTON_DOWN: return sCurrentState.down;
        case SONIC_BUTTON_LEFT: return sCurrentState.left;
        case SONIC_BUTTON_RIGHT: return sCurrentState.right;
        case SONIC_BUTTON_A: return sCurrentState.a;
        case SONIC_BUTTON_B: return sCurrentState.b;
        case SONIC_BUTTON_START: return sCurrentState.start;
        case SONIC_BUTTON_SELECT: return sCurrentState.select;
        default: return false;
    }
}

bool isButtonJustPressed(SonicButton button) {
    bool current = isButtonPressed(button);
    bool previous = false;
    
    switch (button) {
        case SONIC_BUTTON_UP: previous = sPreviousState.up; break;
        case SONIC_BUTTON_DOWN: previous = sPreviousState.down; break;
        case SONIC_BUTTON_LEFT: previous = sPreviousState.left; break;
        case SONIC_BUTTON_RIGHT: previous = sPreviousState.right; break;
        case SONIC_BUTTON_A: previous = sPreviousState.a; break;
        case SONIC_BUTTON_B: previous = sPreviousState.b; break;
        case SONIC_BUTTON_START: previous = sPreviousState.start; break;
        case SONIC_BUTTON_SELECT: previous = sPreviousState.select; break;
        default: previous = false; break;
    }
    
    return current && !previous;
}

bool isButtonJustReleased(SonicButton button) {
    bool current = isButtonPressed(button);
    bool previous = false;
    
    switch (button) {
        case SONIC_BUTTON_UP: previous = sPreviousState.up; break;
        case SONIC_BUTTON_DOWN: previous = sPreviousState.down; break;
        case SONIC_BUTTON_LEFT: previous = sPreviousState.left; break;
        case SONIC_BUTTON_RIGHT: previous = sPreviousState.right; break;
        case SONIC_BUTTON_A: previous = sPreviousState.a; break;
        case SONIC_BUTTON_B: previous = sPreviousState.b; break;
        case SONIC_BUTTON_START: previous = sPreviousState.start; break;
        case SONIC_BUTTON_SELECT: previous = sPreviousState.select; break;
        default: previous = false; break;
    }
    
    return !current && previous;
}

// Sonic 3 specific input helpers
bool isJumpPressed() {
    return isButtonPressed(SONIC_BUTTON_A);
}

bool isJumpJustPressed() {
    return isButtonJustPressed(SONIC_BUTTON_A);
}

bool isSpinDashPressed() {
    return isButtonPressed(SONIC_BUTTON_B);
}

bool isSpinDashJustPressed() {
    return isButtonJustReleased(SONIC_BUTTON_B);
}

bool isStartPressed() {
    return isButtonPressed(SONIC_BUTTON_START);
}

bool isStartJustPressed() {
    return isButtonJustPressed(SONIC_BUTTON_START);
}

// Directional input helpers
float getHorizontalInput() {
    float input = 0.0f;
    
    if (isButtonPressed(SONIC_BUTTON_LEFT)) input -= 1.0f;
    if (isButtonPressed(SONIC_BUTTON_RIGHT)) input += 1.0f;
    
    // Add analog stick input
    input += getLeftStickX();
    
    // Clamp to [-1, 1]
    return std::max(-1.0f, std::min(1.0f, input));
}

float getVerticalInput() {
    float input = 0.0f;
    
    if (isButtonPressed(SONIC_BUTTON_UP)) input -= 1.0f;
    if (isButtonPressed(SONIC_BUTTON_DOWN)) input += 1.0f;
    
    // Add analog stick input
    input += getLeftStickY();
    
    // Clamp to [-1, 1]
    return std::max(-1.0f, std::min(1.0f, input));
}

} // namespace platform_input

#endif // PLATFORM_WIIU
