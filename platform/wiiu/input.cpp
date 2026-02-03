#include "input.h"
#include <vpad/input.h>
#include <coreinit/systeminfo.h>
#include <cstring>
#include <cstdint>

namespace wiiu {

static VPADStatus vpadData;
static VPADReadError vpadError;

void initializeInput() {
    VPADInit();
}

void pollInput(InputState& state) {
    // Clear state
    memset(&state, 0, sizeof(InputState));
    
    // Read GamePad
    VPADRead(VPAD_CHAN_0, &vpadData, 1, &vpadError);
    if (vpadError == VPAD_READ_SUCCESS) {
        // D-pad
        state.up = (vpadData.hold & VPAD_BUTTON_UP) != 0;
        state.down = (vpadData.hold & VPAD_BUTTON_DOWN) != 0;
        state.left = (vpadData.hold & VPAD_BUTTON_LEFT) != 0;
        state.right = (vpadData.hold & VPAD_BUTTON_RIGHT) != 0;
        
        // Face buttons
        state.a = (vpadData.hold & VPAD_BUTTON_A) != 0;
        state.b = (vpadData.hold & VPAD_BUTTON_B) != 0;
        state.x = (vpadData.hold & VPAD_BUTTON_X) != 0;
        state.y = (vpadData.hold & VPAD_BUTTON_Y) != 0;
        
        // System buttons
        state.start = (vpadData.hold & VPAD_BUTTON_PLUS) != 0;
        state.select = (vpadData.hold & VPAD_BUTTON_MINUS) != 0;
        
        // Analog sticks (threshold-based)
        const float threshold = 0.5f;
        if (vpadData.leftStick.y > threshold) state.up = true;
        if (vpadData.leftStick.y < -threshold) state.down = true;
        if (vpadData.leftStick.x < -threshold) state.left = true;
        if (vpadData.leftStick.x > threshold) state.right = true;
    }
}

uint16_t inputStateToOxygenFlags(const InputState& state) {
    uint16_t flags = 0;
    
    if (state.up) flags |= 0x01;    // UP
    if (state.down) flags |= 0x02;  // DOWN
    if (state.left) flags |= 0x04;  // LEFT
    if (state.right) flags |= 0x08; // RIGHT
    
    if (state.a) flags |= 0x10;     // A/Jump
    if (state.b) flags |= 0x20;     // B/Attack
    if (state.x) flags |= 0x40;     // X
    if (state.y) flags |= 0x80;     // Y
    
    if (state.start) flags |= 0x100;    // START
    if (state.select) flags |= 0x200;   // SELECT
    
    return flags;
}

}
