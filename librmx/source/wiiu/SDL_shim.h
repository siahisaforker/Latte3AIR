#pragma once

#include <cstdint>
#include <cstddef>

// Minimal SDL2-compatible shim for PLATFORM_WIIU.
// This is intentionally incomplete and only covers the symbols used by this codebase.

// Basic types
typedef uint8_t  Uint8;
typedef int8_t   Sint8;
typedef uint16_t Uint16;
typedef int16_t  Sint16;
typedef uint32_t Uint32;
typedef int32_t  Sint32;
typedef uint64_t Uint64;
typedef int64_t  Sint64;

typedef int32_t SDL_bool;
#define SDL_FALSE 0
#define SDL_TRUE  1

// Version helpers
#define SDL_MAJOR_VERSION 2
#define SDL_MINOR_VERSION 32
#define SDL_PATCHLEVEL    0
#define SDL_VERSION_ATLEAST(x,y,z) ((SDL_MAJOR_VERSION > (x)) || (SDL_MAJOR_VERSION == (x) && SDL_MINOR_VERSION > (y)) || (SDL_MAJOR_VERSION == (x) && SDL_MINOR_VERSION == (y) && SDL_PATCHLEVEL >= (z)))
#define SDL_arraysize(a) (int)(sizeof(a) / sizeof((a)[0]))

struct SDL_version { Uint8 major, minor, patch; };
#define SDL_VERSION(x) do { (x)->major = SDL_MAJOR_VERSION; (x)->minor = SDL_MINOR_VERSION; (x)->patch = SDL_PATCHLEVEL; } while (0)

// Window / surface
struct SDL_PixelFormat
{
	Uint32 format;
};

struct SDL_Surface
{
	void* pixels = nullptr;
	int w = 0;
	int h = 0;
	int pitch = 0;
	SDL_PixelFormat* format = nullptr;
};

struct SDL_Window
{
	int w;
	int h;
	Uint32 id;
	SDL_Surface* surface;
};

// Pixel formats (values are arbitrary but stable inside this shim)
#define SDL_PIXELFORMAT_UNKNOWN   0
#define SDL_PIXELFORMAT_RGB888    1
#define SDL_PIXELFORMAT_ARGB8888  2
#define SDL_PIXELFORMAT_ABGR8888  3
#define SDL_PIXELFORMAT_BGR888    4
#define SDL_PIXELFORMAT_BGRX8888  5
#define SDL_PIXELFORMAT_RGB565    6
#define SDL_PIXELFORMAT_BGR565    7
#define SDL_PIXELFORMAT_RGBA8888  8
#define SDL_PIXELFORMAT_BGRA8888  9
#define SDL_PIXELFORMAT_RGB24     10
#define SDL_PIXELFORMAT_BGR24     11
#define SDL_PIXELFORMAT_ARGB2101010 12
#define SDL_PIXELFORMAT_RGB444    13
#define SDL_PIXELFORMAT_RGB555    14
#define SDL_PIXELFORMAT_YV12      20
#define SDL_PIXELFORMAT_IYUV      21
#define SDL_PIXELFORMAT_YUY2      22
#define SDL_PIXELFORMAT_UYVY      23
#define SDL_PIXELFORMAT_YVYU      24
#define SDL_PIXELFORMAT_NV12      25
#define SDL_PIXELFORMAT_NV21      26
#define SDL_PIXELFORMAT_RGBX8888  27
#define SDL_PIXELFORMAT_RGBA5551  28
#define SDL_PIXELFORMAT_ABGR1555  29
#define SDL_PIXELFORMAT_BGRA5551  30
#define SDL_PIXELFORMAT_ARGB4444  31
#define SDL_PIXELFORMAT_RGBA4444  32
#define SDL_PIXELFORMAT_ABGR4444  33
#define SDL_PIXELFORMAT_BGRA4444  34
#define SDL_PIXELFORMAT_BGR555    35
#define SDL_PIXELFORMAT_ARGB1555  36
#define SDL_PIXELFORMAT_INDEX1LSB 40
#define SDL_PIXELFORMAT_INDEX1MSB 41
#define SDL_PIXELFORMAT_INDEX4LSB 42
#define SDL_PIXELFORMAT_INDEX4MSB 43
#define SDL_PIXELFORMAT_INDEX8    44
#define SDL_PIXELFORMAT_RGB332    45

// Window flags
#define SDL_WINDOW_OPENGL            0x00000002u
#define SDL_WINDOW_FULLSCREEN        0x00000001u
#define SDL_WINDOW_FULLSCREEN_DESKTOP 0x00001001u
#define SDL_WINDOW_BORDERLESS        0x00000010u
#define SDL_WINDOW_RESIZABLE         0x00000020u

#define SDL_WINDOWPOS_CENTERED 0
#define SDL_WINDOWPOS_CENTERED_DISPLAY(x) 0

// Event types
#define SDL_QUIT                    0x100
#define SDL_WINDOWEVENT             0x200
#define SDL_KEYDOWN                 0x300
#define SDL_KEYUP                   0x301
#define SDL_TEXTINPUT               0x302
#define SDL_MOUSEBUTTONDOWN         0x401
#define SDL_MOUSEBUTTONUP           0x402
#define SDL_MOUSEMOTION             0x403
#define SDL_MOUSEWHEEL              0x404
#define SDL_JOYDEVICEADDED          0x605
#define SDL_JOYDEVICEREMOVED        0x606
#define SDL_APP_WILLENTERBACKGROUND 0x707

#define SDL_WINDOWEVENT_RESIZED      0x05
#define SDL_WINDOWEVENT_SIZE_CHANGED 0x07
#define SDL_WINDOWEVENT_FOCUS_LOST   0x0C

// Keyboard
typedef int32_t SDL_Keycode;
typedef int32_t SDL_Scancode;

#define SDL_NUM_SCANCODES 0x0200

// Common keycodes used in the project
#define SDLK_RETURN       13
#define SDLK_TAB          9
#define SDLK_ESCAPE       27
#define SDLK_BACKSPACE    8
#define SDLK_SPACE        32

// Punctuation / symbols (SDL keycodes match ASCII for these)
#define SDLK_EXCLAIM      '!'
#define SDLK_QUOTEDBL     '"'
#define SDLK_HASH         '#'
#define SDLK_PERCENT      '%'
#define SDLK_DOLLAR       '$'
#define SDLK_AMPERSAND    '&'
#define SDLK_QUOTE        '\''
#define SDLK_LEFTPAREN    '('
#define SDLK_RIGHTPAREN   ')'
#define SDLK_ASTERISK     '*'
#define SDLK_PLUS         '+'
#define SDLK_COMMA        ','
#define SDLK_MINUS        '-'
#define SDLK_PERIOD       '.'
#define SDLK_SLASH        '/'
#define SDLK_0            '0'
#define SDLK_1            '1'
#define SDLK_2            '2'
#define SDLK_3            '3'
#define SDLK_4            '4'
#define SDLK_5            '5'
#define SDLK_6            '6'
#define SDLK_7            '7'
#define SDLK_8            '8'
#define SDLK_9            '9'
#define SDLK_COLON        ':'
#define SDLK_SEMICOLON    ';'
#define SDLK_LESS         '<'
#define SDLK_EQUALS       '='
#define SDLK_GREATER      '>'
#define SDLK_QUESTION     '?'
#define SDLK_AT           '@'
#define SDLK_LEFTBRACKET  '['
#define SDLK_BACKSLASH    '\\'
#define SDLK_RIGHTBRACKET ']'
#define SDLK_CARET        '^'
#define SDLK_UNDERSCORE   '_'
#define SDLK_BACKQUOTE    96

// Letter keys (SDL keycodes match ASCII for letters)
#define SDLK_a 'a'
#define SDLK_b 'b'
#define SDLK_c 'c'
#define SDLK_d 'd'
#define SDLK_e 'e'
#define SDLK_f 'f'
#define SDLK_g 'g'
#define SDLK_h 'h'
#define SDLK_i 'i'
#define SDLK_j 'j'
#define SDLK_k 'k'
#define SDLK_l 'l'
#define SDLK_m 'm'
#define SDLK_n 'n'
#define SDLK_o 'o'
#define SDLK_p 'p'
#define SDLK_q 'q'
#define SDLK_r 'r'
#define SDLK_s 's'
#define SDLK_t 't'
#define SDLK_u 'u'
#define SDLK_v 'v'
#define SDLK_w 'w'
#define SDLK_x 'x'
#define SDLK_y 'y'
#define SDLK_z 'z'
#define SDLK_PRINTSCREEN  0x40000046
#define SDLK_F1           0x4000003A
#define SDLK_F2           0x4000003B
#define SDLK_F3           0x4000003C
#define SDLK_F4           0x4000003D
#define SDLK_F5           0x4000003E
#define SDLK_F7           0x40000040
#define SDLK_F8           0x40000041
#define SDLK_F10          0x40000043
#define SDLK_F11          0x40000044

#define SDLK_HOME         0x4000004A
#define SDLK_PAGEUP       0x4000004B
#define SDLK_END          0x4000004D
#define SDLK_PAGEDOWN     0x4000004E

#define SDLK_RIGHT        0x4000004F
#define SDLK_LEFT         0x40000050
#define SDLK_DOWN         0x40000051
#define SDLK_UP           0x40000052
#define SDLK_KP_PLUS      0x40000057
#define SDLK_KP_MINUS     0x40000056
#define SDLK_KP_DIVIDE    0x40000054
#define SDLK_KP_MULTIPLY  0x40000055
#define SDLK_KP_PERIOD    0x40000063
#define SDLK_KP_ENTER     0x40000058
#define SDLK_KP_0         0x40000062
#define SDLK_KP_1         0x40000059
#define SDLK_KP_2         0x4000005A
#define SDLK_KP_3         0x4000005B
#define SDLK_KP_4         0x4000005C
#define SDLK_KP_5         0x4000005D
#define SDLK_KP_6         0x4000005E
#define SDLK_KP_7         0x4000005F
#define SDLK_KP_8         0x40000060
#define SDLK_KP_9         0x40000061
#define SDLK_LALT         0x400000E2
#define SDLK_RALT         0x400000E6
#define SDLK_LSHIFT       0x400000E1
#define SDLK_RSHIFT       0x400000E5
#define SDLK_LCTRL        0x400000E0
#define SDLK_RCTRL        0x400000E4
#define SDLK_CLEAR        0x4000009C

#define SDLK_CAPSLOCK     0x40000039
#define SDLK_INSERT       0x40000049
#define SDLK_DELETE       127

// SDL encodes scancodes into keycodes using this mask.
#define SDLK_SCANCODE_MASK (1u << 30)

#define KMOD_NONE   0x0000
#define KMOD_LSHIFT 0x0001
#define KMOD_RSHIFT 0x0002
#define KMOD_SHIFT  (KMOD_LSHIFT | KMOD_RSHIFT)
#define KMOD_LCTRL  0x0040
#define KMOD_RCTRL  0x0080
#define KMOD_CTRL   (KMOD_LCTRL | KMOD_RCTRL)
#define KMOD_LALT   0x0100
#define KMOD_RALT   0x0200
#define KMOD_ALT    (KMOD_LALT | KMOD_RALT)

struct SDL_Keysym
{
	SDL_Scancode scancode;
	SDL_Keycode sym;
	Uint16 mod;
};

struct SDL_KeyboardEvent
{
	Uint32 type;
	Uint8 state;
	Uint8 repeat;
	SDL_Keysym keysym;
};

struct SDL_TextInputEvent
{
	Uint32 type;
	char text[32];
};

struct SDL_MouseButtonEvent
{
	Uint32 type;
	Uint8 button;
	Uint8 state;
	Uint8 clicks;
	int x;
	int y;
};

struct SDL_MouseWheelEvent
{
	Uint32 type;
	int x;
	int y;
};

struct SDL_MouseMotionEvent
{
	Uint32 type;
	int x;
	int y;
};

struct SDL_WindowEvent
{
	Uint32 type;
	Uint8 event;
	Uint32 windowID;
	int data1;
	int data2;
};

typedef Sint64 SDL_TouchID;
struct SDL_Finger
{
	SDL_TouchID id;
	float x;
	float y;
	float pressure;
};

struct SDL_Event
{
	Uint32 type;
	union
	{
		SDL_KeyboardEvent key;
		SDL_TextInputEvent text;
		SDL_MouseButtonEvent button;
		SDL_MouseWheelEvent wheel;
		SDL_MouseMotionEvent motion;
		SDL_WindowEvent window;
	};
};

// Joystick / controller
struct SDL_Joystick;
struct SDL_GameController;

enum SDL_GameControllerBindType
{
	SDL_CONTROLLER_BINDTYPE_NONE,
	SDL_CONTROLLER_BINDTYPE_AXIS,
	SDL_CONTROLLER_BINDTYPE_BUTTON,
	SDL_CONTROLLER_BINDTYPE_HAT
};

enum SDL_GameControllerAxis
{
	SDL_CONTROLLER_AXIS_LEFTX = 0,
	SDL_CONTROLLER_AXIS_LEFTY = 1,
	SDL_CONTROLLER_AXIS_RIGHTX = 2,
	SDL_CONTROLLER_AXIS_RIGHTY = 3,
	SDL_CONTROLLER_AXIS_TRIGGERLEFT = 4,
	SDL_CONTROLLER_AXIS_TRIGGERRIGHT = 5
};

enum SDL_GameControllerButton
{
	SDL_CONTROLLER_BUTTON_A = 0,
	SDL_CONTROLLER_BUTTON_B = 1,
	SDL_CONTROLLER_BUTTON_X = 2,
	SDL_CONTROLLER_BUTTON_Y = 3,
	SDL_CONTROLLER_BUTTON_BACK = 4,
	SDL_CONTROLLER_BUTTON_GUIDE = 5,
	SDL_CONTROLLER_BUTTON_START = 6,
	SDL_CONTROLLER_BUTTON_LEFTSTICK = 7,
	SDL_CONTROLLER_BUTTON_RIGHTSTICK = 8,
	SDL_CONTROLLER_BUTTON_LEFTSHOULDER = 9,
	SDL_CONTROLLER_BUTTON_RIGHTSHOULDER = 10,
	SDL_CONTROLLER_BUTTON_DPAD_UP = 11,
	SDL_CONTROLLER_BUTTON_DPAD_DOWN = 12,
	SDL_CONTROLLER_BUTTON_DPAD_LEFT = 13,
	SDL_CONTROLLER_BUTTON_DPAD_RIGHT = 14
};

struct SDL_GameControllerButtonBind
{
	SDL_GameControllerBindType bindType;
	union
	{
		int axis;
		int button;
		struct { int hat; int hat_mask; } hat;
	} value;
};

// Hat constants
#define SDL_HAT_CENTERED 0x00
#define SDL_HAT_UP       0x01
#define SDL_HAT_RIGHT    0x02
#define SDL_HAT_DOWN     0x04
#define SDL_HAT_LEFT     0x08

// Audio
typedef Uint16 SDL_AudioFormat;
typedef Uint32 SDL_AudioDeviceID;

#define AUDIO_S16LSB 0x8010

struct SDL_AudioSpec
{
	int freq;
	SDL_AudioFormat format;
	Uint8 channels;
	Uint16 samples;
	void (*callback)(void* userdata, Uint8* stream, int len);
	void* userdata;
};

struct SDL_AudioCVT
{
	int needed = 0;
	Uint8* buf = nullptr;
	int len = 0;
	int len_cvt = 0;
	int len_mult = 1;
};

#define SDL_AUDIO_PLAYING 1

// RWops
struct SDL_RWops;

#define RW_SEEK_SET 0
#define RW_SEEK_CUR 1
#define RW_SEEK_END 2

// Message boxes
typedef struct SDL_MessageBoxButtonData
{
	Uint32 flags;
	int buttonid;
	const char* text;
} SDL_MessageBoxButtonData;

typedef struct SDL_MessageBoxData
{
	Uint32 flags;
	SDL_Window* window;
	const char* title;
	const char* message;
	int numbuttons;
	const SDL_MessageBoxButtonData* buttons;
	const void* colorScheme;
} SDL_MessageBoxData;

#define SDL_MESSAGEBOX_ERROR       0x00000010
#define SDL_MESSAGEBOX_WARNING     0x00000020
#define SDL_MESSAGEBOX_INFORMATION 0x00000040
#define SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT 0x00000001
#define SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT 0x00000002

// Display
struct SDL_Rect { int x, y, w, h; };
struct SDL_DisplayMode { int w, h; int refresh_rate; void* driverdata; };

// Threading
struct SDL_Thread;
struct SDL_mutex;
struct SDL_cond;

// Hints
#define SDL_HINT_RENDER_VSYNC "SDL_RENDER_VSYNC"
#define SDL_HINT_VIDEO_ALLOW_SCREENSAVER "SDL_VIDEO_ALLOW_SCREENSAVER"
#define SDL_HINT_MOUSE_TOUCH_EVENTS "SDL_MOUSE_TOUCH_EVENTS"
#define SDL_HINT_ACCELEROMETER_AS_JOYSTICK "SDL_ACCELEROMETER_AS_JOYSTICK"

// API
int SDL_Init(Uint32 flags);
int SDL_InitSubSystem(Uint32 flags);
void SDL_Quit();
void SDL_QuitSubSystem(Uint32 flags);
const char* SDL_GetError();

Uint32 SDL_GetTicks();
void SDL_Delay(Uint32 ms);

int SDL_PollEvent(SDL_Event* event);

// Window
SDL_Window* SDL_CreateWindow(const char* title, int x, int y, int w, int h, Uint32 flags);
void SDL_DestroyWindow(SDL_Window* window);
SDL_Surface* SDL_GetWindowSurface(SDL_Window* window);
int SDL_UpdateWindowSurface(SDL_Window* window);
int SDL_LockSurface(SDL_Surface* surface);
void SDL_UnlockSurface(SDL_Surface* surface);
SDL_Surface* SDL_CreateRGBSurfaceFrom(void* pixels, int width, int height, int depth, int pitch, Uint32 Rmask, Uint32 Gmask, Uint32 Bmask, Uint32 Amask);
void SDL_FreeSurface(SDL_Surface* surface);
void SDL_SetWindowIcon(SDL_Window* window, SDL_Surface* icon);
void SDL_GetWindowSize(SDL_Window* window, int* w, int* h);
Uint32 SDL_GetWindowID(SDL_Window* window);
int SDL_GetWindowDisplayIndex(SDL_Window* window);
int SDL_SetWindowFullscreen(SDL_Window* window, Uint32 flags);
void SDL_SetWindowSize(SDL_Window* window, int w, int h);
void SDL_SetWindowPosition(SDL_Window* window, int x, int y);
void SDL_SetWindowResizable(SDL_Window* window, SDL_bool resizable);
void SDL_SetWindowBordered(SDL_Window* window, SDL_bool bordered);
int SDL_ShowCursor(int toggle);
int SDL_SetHint(const char* name, const char* value);
int SDL_setenv(const char* name, const char* value, int overwrite);
int SDL_GetDisplayBounds(int displayIndex, SDL_Rect* rect);
int SDL_GetDesktopDisplayMode(int displayIndex, SDL_DisplayMode* mode);
SDL_bool SDL_IsScreenSaverEnabled();
void SDL_EnableScreenSaver();
void SDL_DisableScreenSaver();

// OpenGL (stubbed)
int SDL_GL_SetAttribute(int attr, int value);
void* SDL_GL_CreateContext(SDL_Window* window);
void SDL_GL_SetSwapInterval(int interval);
void SDL_GL_SwapWindow(SDL_Window* window);
void* SDL_GL_GetCurrentContext();

// Text input
SDL_bool SDL_IsTextInputActive();
void SDL_StartTextInput();
void SDL_StopTextInput();

// Joystick / game controller
int SDL_NumJoysticks();
SDL_Joystick* SDL_JoystickOpen(int device_index);
void SDL_JoystickClose(SDL_Joystick* joystick);
int SDL_JoystickNumButtons(SDL_Joystick* joystick);
int SDL_JoystickNumAxes(SDL_Joystick* joystick);
int SDL_JoystickNumHats(SDL_Joystick* joystick);
Sint16 SDL_JoystickGetAxis(SDL_Joystick* joystick, int axis);
Uint8 SDL_JoystickGetButton(SDL_Joystick* joystick, int button);
Uint8 SDL_JoystickGetHat(SDL_Joystick* joystick, int hat);
int SDL_JoystickInstanceID(SDL_Joystick* joystick);
const char* SDL_JoystickName(SDL_Joystick* joystick);
#if SDL_VERSION_ATLEAST(2, 0, 18)
int SDL_JoystickHasRumble(SDL_Joystick* joystick);
int SDL_JoystickRumble(SDL_Joystick* joystick, Uint16 low_frequency_rumble, Uint16 high_frequency_rumble, Uint32 duration_ms);
#endif

int SDL_GameControllerAddMappingsFromFile(const char* file);
SDL_GameController* SDL_GameControllerOpen(int device_index);
void SDL_GameControllerClose(SDL_GameController* controller);
const char* SDL_GameControllerName(SDL_GameController* controller);
SDL_GameControllerButtonBind SDL_GameControllerGetBindForAxis(SDL_GameController* gamecontroller, SDL_GameControllerAxis axis);
SDL_GameControllerButtonBind SDL_GameControllerGetBindForButton(SDL_GameController* gamecontroller, SDL_GameControllerButton button);
#if SDL_VERSION_ATLEAST(2, 0, 14)
int SDL_GameControllerSetLED(SDL_GameController* gamecontroller, Uint8 red, Uint8 green, Uint8 blue);
#endif

// Touch (unused on Wii U)
int SDL_GetNumTouchDevices();
SDL_TouchID SDL_GetTouchDevice(int index);
int SDL_GetNumTouchFingers(SDL_TouchID touchID);
const SDL_Finger* SDL_GetTouchFinger(SDL_TouchID touchID, int index);

// Audio
SDL_AudioDeviceID SDL_OpenAudioDevice(const char* device, int iscapture, const SDL_AudioSpec* desired, SDL_AudioSpec* obtained, int allowed_changes);
void SDL_CloseAudioDevice(SDL_AudioDeviceID dev);
void SDL_PauseAudioDevice(SDL_AudioDeviceID dev, int pause_on);
int SDL_GetAudioStatus();
int SDL_GetNumAudioDevices(int iscapture);
const char* SDL_GetAudioDeviceName(int index, int iscapture);
void SDL_LockAudioDevice(SDL_AudioDeviceID dev);
void SDL_UnlockAudioDevice(SDL_AudioDeviceID dev);

// WAV loading / conversion
Uint8* SDL_LoadWAV_RW(SDL_RWops* src, int freesrc, SDL_AudioSpec* spec, Uint8** audio_buf, Uint32* audio_len);
Uint8* SDL_LoadWAV(const char* file, SDL_AudioSpec* spec, Uint8** audio_buf, Uint32* audio_len);
void SDL_FreeWAV(Uint8* audio_buf);
int SDL_BuildAudioCVT(SDL_AudioCVT* cvt, SDL_AudioFormat src_format, Uint8 src_channels, int src_rate, SDL_AudioFormat dst_format, Uint8 dst_channels, int dst_rate);
int SDL_ConvertAudio(SDL_AudioCVT* cvt);

// RWops
SDL_RWops* SDL_RWFromFile(const char* file, const char* mode);
SDL_RWops* SDL_RWFromFile(const wchar_t* file, const char* mode);
Sint64 SDL_RWsize(SDL_RWops* context);
Sint64 SDL_RWtell(SDL_RWops* context);
Sint64 SDL_RWseek(SDL_RWops* context, Sint64 offset, int whence);
size_t SDL_RWread(SDL_RWops* context, void* ptr, size_t size, size_t maxnum);
size_t SDL_RWwrite(SDL_RWops* context, const void* ptr, size_t size, size_t num);
int SDL_RWclose(SDL_RWops* context);

// Message boxes / clipboard / URL (stubbed)
int SDL_ShowMessageBox(const SDL_MessageBoxData* messageboxdata, int* buttonid);
int SDL_ShowSimpleMessageBox(Uint32 flags, const char* title, const char* message, SDL_Window* window);
int SDL_OpenURL(const char* url);
int SDL_SetClipboardText(const char* text);
SDL_bool SDL_HasClipboardText();
char* SDL_GetClipboardText();
void SDL_free(void* mem);
void SDL_Log(const char* message);

// Threading
typedef int (*SDL_ThreadFunction)(void* data);
SDL_Thread* SDL_CreateThread(SDL_ThreadFunction fn, const char* name, void* data);
SDL_Thread* SDL_CreateThreadWithStackSize(SDL_ThreadFunction fn, const char* name, size_t stacksize, void* data);
void SDL_WaitThread(SDL_Thread* thread, int* status);

SDL_mutex* SDL_CreateMutex();
void SDL_DestroyMutex(SDL_mutex* mutex);
void SDL_LockMutex(SDL_mutex* mutex);
void SDL_UnlockMutex(SDL_mutex* mutex);

SDL_cond* SDL_CreateCond();
void SDL_DestroyCond(SDL_cond* cond);
void SDL_CondSignal(SDL_cond* cond);
int SDL_CondWait(SDL_cond* cond, SDL_mutex* mutex);
int SDL_CondBroadcast(SDL_cond* cond);
int SDL_CondWaitTimeout(SDL_cond* cond, SDL_mutex* mutex, Uint32 ms);
Uint16 SDL_GetModState();
void SDL_WarpMouseInWindow(SDL_Window* window, int x, int y);

// Performance counter
Uint64 SDL_GetPerformanceCounter();
Uint64 SDL_GetPerformanceFrequency();

// Base / pref path
char* SDL_GetBasePath();
char* SDL_GetPrefPath(const char* org, const char* app);

// Scancode helpers
SDL_Scancode SDL_GetScancodeFromKey(SDL_Keycode key);
const char* SDL_GetKeyName(SDL_Keycode key);
const char* SDL_GetScancodeName(SDL_Scancode scancode);

// Audio mixing
void SDL_MixAudioFormat(Uint8* dst, const Uint8* src, SDL_AudioFormat format, Uint32 len, int volume);
void SDL_MixAudio(Uint8* dst, const Uint8* src, Uint32 len, int volume);

// Display
const char* SDL_GetCurrentVideoDriver();
int SDL_GetCurrentDisplayMode(int displayIndex, SDL_DisplayMode* mode);
int SDL_GetNumVideoDisplays();

// Misc
const char* SDL_GetPlatform();
int SDL_GetCPUCount();

// Init flags
#define SDL_INIT_VIDEO    0x00000020u
#define SDL_INIT_AUDIO    0x00000010u
#define SDL_INIT_JOYSTICK 0x00000200u

// GL attributes (stub)
#define SDL_GL_RED_SIZE              0
#define SDL_GL_GREEN_SIZE            1
#define SDL_GL_BLUE_SIZE             2
#define SDL_GL_DEPTH_SIZE            3
#define SDL_GL_DOUBLEBUFFER          4
#define SDL_GL_MULTISAMPLESAMPLES    5
#define SDL_GL_CONTEXT_PROFILE_MASK  6
#define SDL_GL_CONTEXT_PROFILE_CORE  1
#define SDL_GL_CONTEXT_PROFILE_ES    2
#define SDL_GL_CONTEXT_MAJOR_VERSION 7
#define SDL_GL_CONTEXT_MINOR_VERSION 8

typedef void* SDL_GLContext;

// Button/Key states
#define SDL_PRESSED 1

