#pragma once
#include "input_state.h"
#include <cstdint>

namespace input {

/// Initialise VPAD / KPAD subsystems.
void initialize_wiiu_input();

/// Poll all connected controllers, updating `states`.
/// Returns count of active controller inputs stored.
int poll_wiiu_input(InputState* states, int maxControllers);

/// Convenience: poll GamePad only.
void poll_wiiu_input();

/// Get the most-recent GamePad state.
const InputState& get_gamepad_state();

/// Check if Pro Controller `index` (0-3) is connected.
bool is_pro_controller_connected(int index);

/// Shut down input subsystem.
void shutdown_wiiu_input();

} // namespace input
