#pragma once

#if defined(PLATFORM_WIIU) || defined(__WIIU__)

#include <cstdint>
#include <cstdlib>

// SDL type definitions for Wii U
typedef uint32_t Uint32;
typedef uint16_t Uint16;
typedef uint8_t Uint8;
typedef int16_t Sint16;
typedef int SDL_JoystickID;

// SDL flags
#define SDL_INIT_TIMER       0x00000001
#define SDL_INIT_AUDIO       0x00000010
#define SDL_INIT_VIDEO       0x00000020
#define SDL_INIT_JOYSTICK    0x00000200
#define SDL_INIT_EVERYTHING  0x0000FFFF

// SDL structures (simplified for Wii U)
struct SDL_AudioSpec {
    int freq;
    Uint16 format;
    Uint8 channels;
    Uint8 silence;
    Uint16 samples;
    void* userdata;
};

struct SDL_Window {
    void* handle;
    int width;
    int height;
};

struct SDL_Renderer {
    void* handle;
};

struct SDL_Texture {
    void* handle;
    Uint32 format;
    int access;
    int width;
    int height;
};

struct SDL_Joystick {
    void* handle;
};

struct SDL_Rect {
    int x, y;
    int w, h;
};

struct SDL_Event {
    Uint32 type;
    // Simplified event structure
    union {
        struct {
            Uint8 type;
            Uint8 state;
            Uint8 which;
        } button;
        struct {
            Uint8 type;
            Uint8 which;
            Sint16 value;
            Uint16 padding;
        } axis;
    } data;
};

// SDL event types
#define SDL_QUIT 0x100
#define SDL_KEYDOWN 0x300
#define SDL_KEYUP 0x301
#define SDL_JOYBUTTONDOWN 0x603
#define SDL_JOYBUTTONUP 0x604
#define SDL_JOYAXISMOTION 0x606

// SDL texture formats
#define SDL_PIXELFORMAT_RGBA8888 0x16462004

// SDL texture access
#define SDL_TEXTUREACCESS_STREAMING 2

// SDL surface flags
#define SDL_WINDOW_SHOWN 0x00000004

namespace SDL {

// Function declarations (implemented in platform_sdl.cpp)
int SDL_Init(Uint32 flags);
void SDL_Quit();
Uint32 SDL_GetTicks();
void SDL_Delay(Uint32 ms);

int SDL_OpenAudio(SDL_AudioSpec* desired, SDL_AudioSpec* obtained);
void SDL_PauseAudio(int pause_on);
void SDL_CloseAudio();

int SDL_NumJoysticks();
SDL_Joystick* SDL_JoystickOpen(int device_index);
void SDL_JoystickClose(SDL_Joystick* joystick);
SDL_JoystickID SDL_JoystickInstanceID(SDL_Joystick* joystick);
int SDL_JoystickNumButtons(SDL_Joystick* joystick);
Uint8 SDL_JoystickGetButton(SDL_Joystick* joystick, int button);
int SDL_JoystickNumAxes(SDL_Joystick* joystick);
Sint16 SDL_JoystickGetAxis(SDL_Joystick* joystick, int axis);

int SDL_PollEvent(SDL_Event* event);

int SDL_CreateWindowAndRenderer(SDL_Window** window, SDL_Renderer** renderer,
                                   const char* title, int x, int y, int w, int h,
                                   Uint32 flags);
void SDL_DestroyRenderer(SDL_Renderer* renderer);
void SDL_DestroyWindow(SDL_Window* window);
void SDL_RenderClear(SDL_Renderer* renderer);
void SDL_RenderPresent(SDL_Renderer* renderer);
int SDL_RenderCopy(SDL_Renderer* renderer, SDL_Texture* texture,
                   const SDL_Rect* srcrect, const SDL_Rect* dstrect);
SDL_Texture* SDL_CreateTexture(SDL_Renderer* renderer, Uint32 format,
                              int access, int w, int h);
void SDL_DestroyTexture(SDL_Texture* texture);
void SDL_UpdateTexture(SDL_Texture* texture, const SDL_Rect* rect,
                        const void* pixels, int pitch);

const char* SDL_GetError();
void SDL_ClearError();

} // namespace SDL

#endif // PLATFORM_WIIU
