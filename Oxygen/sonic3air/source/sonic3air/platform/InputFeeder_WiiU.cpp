/*
 * Wii U InputFeeder: VPAD (GamePad) + KPAD (Pro Controller).
 * Poll once per frame, feed into Oxygen via ControlsIn::injectInput().
 */

#include "sonic3air/pch.h"
#include "oxygen/application/input/InputManager.h"
#include "oxygen/application/input/ControlsIn.h"

#if defined(PLATFORM_WIIU)

#include "platform/wiiu/input/input_state.h"

class WiiUInputFeeder : public InputFeeder
{
public:
    WiiUInputFeeder() = default;
    ~WiiUInputFeeder() override = default;

    void updateControls() override
    {
        if (!ControlsIn::hasInstance())
            return;

        InputState state;
        pollWiiUInput(state);
        const uint16_t flags = inputStateToOxygenFlags(state);
        ControlsIn::instance().injectInput(0, flags);
    }
};

static WiiUInputFeeder g_wiiu_input_feeder;

void registerWiiUInputFeeder(InputManager& inputManager)
{
    g_wiiu_input_feeder.registerAtInputManager(inputManager);
}

#endif // PLATFORM_WIIU
