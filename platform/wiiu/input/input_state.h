#pragma once
#include <cstdint>

/// Hardware input state snapshot from Wii U controllers.
struct InputState {
    // D-pad + face buttons (matching VPAD_BUTTON_* bits)
    uint32_t buttonsHeld    = 0;
    uint32_t buttonsPressed = 0;  // newly pressed this frame
    uint32_t buttonsReleased = 0; // released this frame

    // Analog sticks (normalized –1..+1)
    float leftStickX  = 0.0f;
    float leftStickY  = 0.0f;
    float rightStickX = 0.0f;
    float rightStickY = 0.0f;

    // Analog triggers (0..1)
    float triggerL = 0.0f;
    float triggerR = 0.0f;

    // Touch screen (DRC only, normalised 0..1)
    bool touchActive    = false;
    float touchX        = 0.0f;
    float touchY        = 0.0f;

    // Which controller index supplied this state (0 = GamePad, 1-4 = Pro Controller)
    int controllerIndex = 0;
};

/// Button flag constants (map from VPAD_BUTTON_*)
namespace WiiUButton {
    static constexpr uint32_t A      = 0x8000;
    static constexpr uint32_t B      = 0x4000;
    static constexpr uint32_t X      = 0x2000;
    static constexpr uint32_t Y      = 0x1000;
    static constexpr uint32_t Left   = 0x0800;
    static constexpr uint32_t Right  = 0x0400;
    static constexpr uint32_t Up     = 0x0200;
    static constexpr uint32_t Down   = 0x0100;
    static constexpr uint32_t ZL     = 0x0080;
    static constexpr uint32_t ZR     = 0x0040;
    static constexpr uint32_t L      = 0x0020;
    static constexpr uint32_t R      = 0x0010;
    static constexpr uint32_t Plus   = 0x0008;
    static constexpr uint32_t Minus  = 0x0004;
    static constexpr uint32_t Home   = 0x0002;
    static constexpr uint32_t Sync   = 0x0001;
    static constexpr uint32_t StickL = 0x00040000;
    static constexpr uint32_t StickR = 0x00020000;
}

/// Poll VPAD and fill InputState for the GamePad.
void pollWiiUInput(InputState& state);

/// Convert InputState button flags to engine-compatible 16-bit oxygen input flags.
uint16_t inputStateToOxygenFlags(const InputState& state);
