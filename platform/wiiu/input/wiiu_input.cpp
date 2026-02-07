#include "wiiu_input.h"

#if defined(PLATFORM_WIIU)

#include <vpad/input.h>
#include <padscore/kpad.h>
#include <cstring>

namespace {
    InputState g_gamepadState;
    InputState g_proStates[4];
    bool g_proConnected[4] = {false};
    uint32_t g_prevButtons = 0;

    float clampAxis(float v) { return v < -1.0f ? -1.0f : (v > 1.0f ? 1.0f : v); }
}

void pollWiiUInput(InputState& state)
{
    VPADStatus status{};
    VPADReadError error = VPAD_READ_NO_SAMPLES;
    VPADRead(VPAD_CHAN_0, &status, 1, &error);
    if (error != VPAD_READ_SUCCESS)
    {
        std::memset(&status, 0, sizeof(status));
    }

    uint32_t held = status.hold;
    state.buttonsPressed  = held & ~g_prevButtons;
    state.buttonsReleased = g_prevButtons & ~held;
    state.buttonsHeld     = held;
    g_prevButtons = held;

    state.leftStickX  = clampAxis(status.leftStick.x);
    state.leftStickY  = clampAxis(status.leftStick.y);
    state.rightStickX = clampAxis(status.rightStick.x);
    state.rightStickY = clampAxis(status.rightStick.y);

    // Wii U GamePad has digital triggers exposed as buttons
    state.triggerL = (held & VPAD_BUTTON_ZL) ? 1.0f : 0.0f;
    state.triggerR = (held & VPAD_BUTTON_ZR) ? 1.0f : 0.0f;

    // Touch screen
    VPADTouchData tp{};
    VPADGetTPCalibratedPoint(VPAD_CHAN_0, &tp, &status.tpNormal);
    state.touchActive = (tp.touched != 0);
    if (state.touchActive)
    {
        state.touchX = static_cast<float>(tp.x) / 1280.0f;
        state.touchY = static_cast<float>(tp.y) / 720.0f;
    }
    else
    {
        state.touchX = 0.0f;
        state.touchY = 0.0f;
    }

    state.controllerIndex = 0;
}

uint16_t inputStateToOxygenFlags(const InputState& state)
{
    // Oxygen engine input flags (from oxygen/application/input/InputManager.h):
    //  Bit 0: Up, 1: Down, 2: Left, 3: Right, 4: A, 5: B, 6: X, 7: Y
    //  Bit 8: Start, 9: Back/Select, 10: L, 11: R
    uint16_t flags = 0;
    uint32_t h = state.buttonsHeld;
    if (h & VPAD_BUTTON_UP)    flags |= (1 << 0);
    if (h & VPAD_BUTTON_DOWN)  flags |= (1 << 1);
    if (h & VPAD_BUTTON_LEFT)  flags |= (1 << 2);
    if (h & VPAD_BUTTON_RIGHT) flags |= (1 << 3);
    if (h & VPAD_BUTTON_A)     flags |= (1 << 4);
    if (h & VPAD_BUTTON_B)     flags |= (1 << 5);
    if (h & VPAD_BUTTON_X)     flags |= (1 << 6);
    if (h & VPAD_BUTTON_Y)     flags |= (1 << 7);
    if (h & VPAD_BUTTON_PLUS)  flags |= (1 << 8);
    if (h & VPAD_BUTTON_MINUS) flags |= (1 << 9);
    if (h & VPAD_BUTTON_L)     flags |= (1 << 10);
    if (h & VPAD_BUTTON_R)     flags |= (1 << 11);
    if (h & VPAD_BUTTON_ZL)    flags |= (1 << 10);
    if (h & VPAD_BUTTON_ZR)    flags |= (1 << 11);

    // Analog stick → D-pad fallback
    if (state.leftStickY >  0.5f) flags |= (1 << 0);
    if (state.leftStickY < -0.5f) flags |= (1 << 1);
    if (state.leftStickX < -0.5f) flags |= (1 << 2);
    if (state.leftStickX >  0.5f) flags |= (1 << 3);

    return flags;
}

namespace input {

void initialize_wiiu_input()
{
    VPADInit();
    KPADInit();
}

int poll_wiiu_input(InputState* states, int maxControllers)
{
    if (!states || maxControllers <= 0) return 0;

    // GamePad
    pollWiiUInput(g_gamepadState);
    states[0] = g_gamepadState;
    int count = 1;

    // Pro Controllers (up to 4)
    for (int i = 0; i < 4 && count < maxControllers; ++i)
    {
        KPADStatus kpadStatus{};
        int32_t err = 0;
        int32_t read = KPADRead(static_cast<KPADChan>(i), &kpadStatus, 1);
        if (read <= 0)
        {
            g_proConnected[i] = false;
            continue;
        }

        // Only handle Wii U Pro Controller type
        if (kpadStatus.extensionType != WPAD_EXT_PRO_CONTROLLER)
        {
            g_proConnected[i] = false;
            continue;
        }

        g_proConnected[i] = true;
        InputState& s = g_proStates[i];
        auto& pro = kpadStatus.pro;
        s.buttonsHeld = pro.hold;
        s.buttonsPressed = pro.trigger;
        s.buttonsReleased = pro.release;
        s.leftStickX  = clampAxis(pro.leftStick.x);
        s.leftStickY  = clampAxis(pro.leftStick.y);
        s.rightStickX = clampAxis(pro.rightStick.x);
        s.rightStickY = clampAxis(pro.rightStick.y);
        s.triggerL = 0.0f; // Pro Controller has digital only
        s.triggerR = 0.0f;
        s.touchActive = false;
        s.controllerIndex = i + 1;
        states[count++] = s;
    }

    return count;
}

void poll_wiiu_input()
{
    pollWiiUInput(g_gamepadState);
}

const InputState& get_gamepad_state()
{
    return g_gamepadState;
}

bool is_pro_controller_connected(int index)
{
    if (index < 0 || index >= 4) return false;
    return g_proConnected[index];
}

void shutdown_wiiu_input()
{
    KPADShutdown();
}

} // namespace input

#else

// Non-Wii U stubs
void pollWiiUInput(InputState& state) { (void)state; }
uint16_t inputStateToOxygenFlags(const InputState& state) { (void)state; return 0; }

namespace input {
void initialize_wiiu_input() {}
int poll_wiiu_input(InputState*, int) { return 0; }
void poll_wiiu_input() {}
const InputState& get_gamepad_state() { static InputState s; return s; }
bool is_pro_controller_connected(int) { return false; }
void shutdown_wiiu_input() {}
} // namespace input

#endif
