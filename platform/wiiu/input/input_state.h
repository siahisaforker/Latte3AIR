#pragma once

#include <cstdint>
#include <cstddef>

/**
 * Unified Wii U input state.
 * VPAD (GamePad) + KPAD (Pro Controller / Wiimote).
 * Poll once per frame, feed into Oxygen via ControlsIn::injectInput().
 */

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
    bool select = false;  // Back / Minus
    bool l = false;
    bool r = false;

    /** Left stick X: -1..1 (negative = left). Used for d-pad fallback. */
    float leftStickX = 0.0f;
    /** Left stick Y: -1..1 (negative = up). Used for d-pad fallback. */
    float leftStickY = 0.0f;
};

/** Analog threshold for stick -> d-pad fallback. */
constexpr float kStickToDpadThreshold = 0.5f;

/**
 * Poll VPAD (and optionally KPAD), merge into a single InputState.
 * Prefer VPAD (GamePad); if no VPAD sample, use KPAD channel 0 (Pro Controller).
 */
void pollWiiUInput(InputState& out);

/**
 * Convert InputState to Oxygen ControlsIn button flags (uint16).
 * Maps: up/down/left/right, a/b/x/y, start -> START, select -> MODE, l -> Z, r -> R.
 * Stick beyond threshold adds d-pad bits.
 */
uint16_t inputStateToOxygenFlags(const InputState& state);
