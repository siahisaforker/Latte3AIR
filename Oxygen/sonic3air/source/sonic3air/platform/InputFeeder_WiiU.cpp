/*
 * Wii U InputFeeder stub
 * Registers at the InputManager and provides TODO hooks to poll WHB pad and Pro controller state.
 */

#include "sonic3air/pch.h"
#include "oxygen/application/input/InputManager.h"

#if defined(PLATFORM_WIIU)

class WiiUInputFeeder : public InputFeeder
{
public:
    WiiUInputFeeder() = default;
    ~WiiUInputFeeder() override {}

    void updateControls() override
    {
        if (!mInputManager)
            return;

        // TODO: Query WHBPad or other WUT input APIs and feed InputManager via its public methods.
        // Example placeholder behavior: do nothing for now so the app can build and run.
    }
};

static WiiUInputFeeder g_wiiu_input_feeder;

void registerWiiUInputFeeder(InputManager& inputManager)
{
    g_wiiu_input_feeder.registerAtInputManager(inputManager);
}

#endif // PLATFORM_WIIU
