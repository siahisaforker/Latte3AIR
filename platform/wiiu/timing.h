#pragma once

#include <cstdint>

namespace wiiu {

// Timer functions
bool initializeTimer();
void shutdownTimer();
bool initializeVideo();
void shutdownVideo();

uint32_t getTicks();
void sleep(uint32_t ms);

// Audio functions
bool openAudio(void* desired, void* obtained);
void pauseAudio(int pause_on);
void closeAudio();

// Joystick functions
int numJoysticks();
void* joystickOpen(int device_index);
void joystickClose(void* joystick);
int joystickInstanceID(void* joystick);
int joystickNumButtons(void* joystick);
uint8_t joystickGetButton(void* joystick, int button);
int joystickNumAxes(void* joystick);
int16_t joystickGetAxis(void* joystick, int axis);

// Event functions
int pollEvent(void* event);

// Video functions
int createWindowAndRenderer(void** window, void** renderer,
                           const char* title, int x, int y, int w, int h,
                           uint32_t flags);
void destroyRenderer(void* renderer);
void destroyWindow(void* window);
void renderClear(void* renderer);
void renderPresent(void* renderer);
int renderCopy(void* renderer, void* texture,
               const void* srcrect, const void* dstrect);
void* createTexture(void* renderer, uint32_t format,
                    int access, int w, int h);
void destroyTexture(void* texture);
void updateTexture(void* texture, const void* rect,
                    const void* pixels, int pitch);

// Error handling
const char* getLastError();
void clearError();

}
