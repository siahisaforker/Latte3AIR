#include "input_state.h"

#if defined(PLATFORM_WIIU) || defined(__WIIU__)

#include <vpad/input.h>
#include <cstring>

#if defined(__has_include)
#if __has_include(<padscore/kpad.h>)
#define WIIU_HAS_KPAD 1
#endif
#endif

#if defined(WIIU_HAS_KPAD)
#include <padscore/kpad.h>
#endif

namespace {

void applyVpadToState(const VPADStatus& status, InputState& out) {
    const uint32_t hold = status.hold;

    out.up    = (hold & VPAD_BUTTON_UP) != 0;
    out.down  = (hold & VPAD_BUTTON_DOWN) != 0;
    out.left  = (hold & VPAD_BUTTON_LEFT) != 0;
    out.right = (hold & VPAD_BUTTON_RIGHT) != 0;
    out.a     = (hold & VPAD_BUTTON_A) != 0;
    out.b     = (hold & VPAD_BUTTON_B) != 0;
    out.x     = (hold & VPAD_BUTTON_X) != 0;
    out.y     = (hold & VPAD_BUTTON_Y) != 0;
    out.start = (hold & VPAD_BUTTON_PLUS) != 0;
    out.select = (hold & VPAD_BUTTON_MINUS) != 0;
    out.l     = (hold & VPAD_BUTTON_L) != 0;
    out.r     = (hold & VPAD_BUTTON_R) != 0;

    out.leftStickX = status.leftStick.x;
    out.leftStickY = status.leftStick.y;
}

#if defined(WIIU_HAS_KPAD)
void applyKpadToState(const KPADStatus& status, InputState& out) {
    uint32_t hold = status.hold;
    if (status.extensionType == WPAD_EXT_PRO_CONTROLLER) {
        hold = status.pro.hold;
        out.leftStickX = status.pro.leftStick.x;
        out.leftStickY = status.pro.leftStick.y;
    } else if (status.extensionType == WPAD_EXT_CLASSIC) {
        hold |= status.classic.hold;
        out.leftStickX = status.classic.leftStick.x;
        out.leftStickY = status.classic.leftStick.y;
    }

    out.up    = (hold & WPAD_BUTTON_UP) != 0;
    out.down  = (hold & WPAD_BUTTON_DOWN) != 0;
    out.left  = (hold & WPAD_BUTTON_LEFT) != 0;
    out.right = (hold & WPAD_BUTTON_RIGHT) != 0;
    out.a     = (hold & WPAD_BUTTON_A) != 0;
    out.b     = (hold & WPAD_BUTTON_B) != 0;
    out.x     = (hold & WPAD_BUTTON_1) != 0;
    out.y     = (hold & WPAD_BUTTON_2) != 0;
    out.start = (hold & WPAD_BUTTON_PLUS) != 0;
    out.select = (hold & WPAD_BUTTON_MINUS) != 0;
    out.l     = (hold & WPAD_BUTTON_ZL) != 0;
    out.r     = (hold & WPAD_BUTTON_ZR) != 0;
}
#endif

} // namespace

void pollWiiUInput(InputState& out) {
    std::memset(&out, 0, sizeof(InputState));

    VPADStatus vpad{};
    VPADReadError vpadError = VPAD_READ_NO_SAMPLES;
    VPADRead(VPAD_CHAN_0, &vpad, 1, &vpadError);

    if (vpadError == VPAD_READ_SUCCESS) {
        applyVpadToState(vpad, out);
        return;
    }

#if defined(WIIU_HAS_KPAD)
    KPADStatus kpad{};
    KPADError kpadError = KPAD_ERROR_NO_SAMPLES;
    if (KPADReadEx(KPAD_CHAN_0, &kpad, 1, &kpadError) > 0 && kpadError == KPAD_ERROR_OK) {
        applyKpadToState(kpad, out);
    }
#endif
}

uint16_t inputStateToOxygenFlags(const InputState& state) {
    // Oxygen ControlsIn::Button: UP=0x0001, DOWN=0x0002, LEFT=0x0004, RIGHT=0x0008,
    // B=0x0010, C=0x0020, A=0x0040, START=0x0080, Z=0x0100, Y=0x0200, X=0x0400, MODE=0x0800
    constexpr uint16_t UP    = 0x0001;
    constexpr uint16_t DOWN  = 0x0002;
    constexpr uint16_t LEFT  = 0x0004;
    constexpr uint16_t RIGHT = 0x0008;
    constexpr uint16_t B     = 0x0010;
    constexpr uint16_t C     = 0x0020;
    constexpr uint16_t A     = 0x0040;
    constexpr uint16_t START = 0x0080;
    constexpr uint16_t Z     = 0x0100;
    constexpr uint16_t Y     = 0x0200;
    constexpr uint16_t X     = 0x0400;
    constexpr uint16_t MODE  = 0x0800;

    uint16_t flags = 0;

    if (state.up)    flags |= UP;
    if (state.down)  flags |= DOWN;
    if (state.left)  flags |= LEFT;
    if (state.right) flags |= RIGHT;
    if (state.a)     flags |= A;
    if (state.b)     flags |= B;
    if (state.x)     flags |= X;
    if (state.y)     flags |= Y;
    if (state.start)  flags |= START;
    if (state.select) flags |= MODE;
    if (state.l)      flags |= X;   /* L -> X in Oxygen ControllerScheme */
    if (state.r)      flags |= Z;   /* R -> Z in Oxygen ControllerScheme */

    /* Analog stick -> d-pad fallback (only if d-pad not pressed) */
    if (!state.up && !state.down && state.leftStickY < -kStickToDpadThreshold)
        flags |= UP;
    if (!state.down && !state.up && state.leftStickY > kStickToDpadThreshold)
        flags |= DOWN;
    if (!state.left && !state.right && state.leftStickX < -kStickToDpadThreshold)
        flags |= LEFT;
    if (!state.right && !state.left && state.leftStickX > kStickToDpadThreshold)
        flags |= RIGHT;

    return flags;
}

#else

void pollWiiUInput(InputState& out) {
    (void)out;
}

uint16_t inputStateToOxygenFlags(const InputState& state) {
    (void)state;
    return 0;
}

#endif
