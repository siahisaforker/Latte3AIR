#include "platform_sdl.h"

#if defined(PLATFORM_WIIU) || defined(__WIIU__)

#include "../platform/wiiu/input.h"
#include "../platform/wiiu/audio.h"
#include "../platform/wiiu/threading.h"
#include "../platform/wiiu/timing.h"
#include "../platform/wiiu/filesystem.h"
#include "../platform/wiiu/render/renderer.h"
#include "platform_input.h"
#include "platform_audio.h"
#include "platform_threading.h"
#include "platform_timing.h"
#include "platform_filesystem.h"
#include <cstring>
#include <algorithm>

// Forward declarations for missing wiiu functions
namespace wiiu {
    struct Rect {
        int x;
        int y;
        int width;
        int height;
    };
    
    Renderer* getWiiURenderer();
}

// Complete SDL implementation for Wii U with GX2/OSScreen integration
namespace SDL {

// Global state
static bool gSDLInitialized = false;
static SDL_Window* gMainWindow = nullptr;
static SDL_Renderer* gMainRenderer = nullptr;
static SDL_AudioSpec gAudioSpec;
static bool gAudioPaused = true;
static uint32_t gInitFlags = 0;

// Initialize all SDL subsystems
int SDL_Init(Uint32 flags) {
    if (gSDLInitialized) return 0;
    
    gInitFlags = flags;
    
    // Initialize filesystem first
    if (!platform_filesystem::initializeFilesystem()) {
        return -1;
    }
    
    // Initialize audio if requested
    if (flags & SDL_INIT_AUDIO) {
        if (!wiiu::initializeAudio()) {
            return -1;
        }
        gAudioPaused = false;
    }
    
    // Initialize timer if requested
    if (flags & SDL_INIT_TIMER) {
        if (!wiiu::initializeTimer()) {
            return -1;
        }
    }
    
    // Initialize video if requested
    if (flags & SDL_INIT_VIDEO) {
        if (!wiiu::initializeVideo()) {
            return -1;
        }
    }
    
    // Initialize input if requested
    if (flags & SDL_INIT_JOYSTICK) {
        if (!platform_input::initializeInput()) {
            return -1;
        }
    }
    
    gSDLInitialized = true;
    return 0;
}

void SDL_Quit() {
    if (!gSDLInitialized) return;
    
    // Cleanup in reverse order
    if (gMainRenderer) {
        SDL_DestroyRenderer(gMainRenderer);
        gMainRenderer = nullptr;
    }
    
    if (gMainWindow) {
        SDL_DestroyWindow(gMainWindow);
        gMainWindow = nullptr;
    }
    
    if (gInitFlags & SDL_INIT_JOYSTICK) {
        platform_input::shutdownInput();
    }
    
    if (gInitFlags & SDL_INIT_AUDIO) {
        wiiu::shutdownAudio();
    }
    
    if (gInitFlags & SDL_INIT_TIMER) {
        wiiu::shutdownTimer();
    }
    
    if (gInitFlags & SDL_INIT_VIDEO) {
        wiiu::shutdownVideo();
    }
    
    platform_filesystem::shutdownFilesystem();
    
    gSDLInitialized = false;
    gInitFlags = 0;
}

// Timer functions
Uint32 SDL_GetTicks() {
    return wiiu::getTicks();
}

void SDL_Delay(Uint32 ms) {
    wiiu::sleep(ms);
}

// Audio functions
int SDL_OpenAudio(SDL_AudioSpec* desired, SDL_AudioSpec* obtained) {
    if (!desired || !obtained) return -1;
    
    // Store desired spec
    gAudioSpec = *desired;
    
    // Configure Wii U audio system
    // Set up audio parameters for Wii U
    gAudioSpec.freq = desired->freq;
    gAudioSpec.format = AUDIO_S16LSB; // Force 16-bit signed little-endian
    gAudioSpec.channels = desired->channels;
    gAudioSpec.samples = desired->samples;
    
    // Copy to obtained
    *obtained = gAudioSpec;
    
    return 0; // Success
}

void SDL_PauseAudio(int pause_on) {
    gAudioPaused = (pause_on != 0);
    
    if (pause_on) {
        wiiu::stopAllSounds();
    }
}

void SDL_CloseAudio() {
    wiiu::shutdownAudio();
    gAudioPaused = true;
}

// Input functions
int SDL_NumJoysticks() {
    return platform_input::numJoysticks();
}

SDL_Joystick* SDL_JoystickOpen(int device_index) {
    return platform_input::joystickOpen(device_index);
}

void SDL_JoystickClose(SDL_Joystick* joystick) {
    platform_input::joystickClose(joystick);
}

SDL_JoystickID SDL_JoystickInstanceID(SDL_Joystick* joystick) {
    return platform_input::joystickInstanceID(joystick);
}

int SDL_JoystickNumButtons(SDL_Joystick* joystick) {
    return platform_input::joystickNumButtons(joystick);
}

Uint8 SDL_JoystickGetButton(SDL_Joystick* joystick, int button) {
    return platform_input::joystickGetButton(joystick, button);
}

int SDL_JoystickNumAxes(SDL_Joystick* joystick) {
    return platform_input::joystickNumAxes(joystick);
}

Sint16 SDL_JoystickGetAxis(SDL_Joystick* joystick, int axis) {
    return platform_input::joystickGetAxis(joystick, axis);
}

// Event functions
int SDL_PollEvent(SDL_Event* event) {
    if (!event) return 0;
    
    // Clear event
    memset(event, 0, sizeof(SDL_Event));
    
    // Poll input and convert to SDL events
    return platform_input::pollEvent(event);
}

// Video functions with GX2/OSScreen integration
int SDL_CreateWindowAndRenderer(SDL_Window** window, SDL_Renderer** renderer,
                                   const char* title, int x, int y, int w, int h,
                                   Uint32 flags) {
    if (!window || !renderer) return -1;
    
    // Create window
    *window = new SDL_Window();
    (*window)->handle = nullptr; // Handle managed by renderer
    
    // Create renderer
    *renderer = new SDL_Renderer();
    
    // Initialize renderer with GX2/OSScreen
    wiiu::Renderer* wiiuRenderer = wiiu::getWiiURenderer();
    if (!wiiuRenderer) {
        delete *window;
        delete *renderer;
        *window = nullptr;
        *renderer = nullptr;
        return -1;
    }
    
    // Store renderer handle
    (*renderer)->handle = wiiuRenderer;
    
    // Set window properties
    (*window)->width = w;
    (*window)->height = h;
    
    // Store global references
    gMainWindow = *window;
    gMainRenderer = *renderer;
    
    return 0;
}

void SDL_DestroyRenderer(SDL_Renderer* renderer) {
    if (!renderer) return;
    
    if (renderer == gMainRenderer) {
        gMainRenderer = nullptr;
    }
    
    wiiu::Renderer* wiiuRenderer = static_cast<wiiu::Renderer*>(renderer->handle);
    if (wiiuRenderer) {
        wiiuRenderer->shutdown();
    }
    
    delete renderer;
}

void SDL_DestroyWindow(SDL_Window* window) {
    if (!window) return;
    
    if (window == gMainWindow) {
        gMainWindow = nullptr;
    }
    
    delete window;
}

void SDL_RenderClear(SDL_Renderer* renderer) {
    if (!renderer) return;
    
    wiiu::Renderer* wiiuRenderer = static_cast<wiiu::Renderer*>(renderer->handle);
    if (wiiuRenderer) {
        wiiuRenderer->beginFrame();
    }
}

void SDL_RenderPresent(SDL_Renderer* renderer) {
    if (!renderer) return;
    
    wiiu::Renderer* wiiuRenderer = static_cast<wiiu::Renderer*>(renderer->handle);
    if (wiiuRenderer) {
        wiiuRenderer->present();
    }
}

int SDL_RenderCopy(SDL_Renderer* renderer, SDL_Texture* texture,
                   const SDL_Rect* srcrect, const SDL_Rect* dstrect) {
    if (!renderer || !texture) return -1;
    
    wiiu::Renderer* wiiuRenderer = static_cast<wiiu::Renderer*>(renderer->handle);
    if (!wiiuRenderer) return -1;
    
    // Get texture data
    void* textureData = texture->handle;
    if (!textureData) return -1;
    
    // For now, just upload the texture data to the framebuffer
    // TODO: Implement proper texture rendering with src/dst rects
    wiiuRenderer->uploadFrameBuffer(textureData, texture->width, texture->height, texture->width * 4);
    
    return 0;
}

SDL_Texture* SDL_CreateTexture(SDL_Renderer* renderer, Uint32 format,
                              int access, int w, int h) {
    if (!renderer) return nullptr;
    
    // Create a simple texture buffer
    void* textureData = malloc(w * h * 4); // RGBA8888
    if (!textureData) return nullptr;
    
    SDL_Texture* texture = new SDL_Texture();
    texture->handle = textureData;
    texture->format = format;
    texture->access = access;
    texture->width = w;
    texture->height = h;
    
    return texture;
}

void SDL_DestroyTexture(SDL_Texture* texture) {
    if (!texture) return;
    
    if (texture->handle) {
        free(texture->handle);
    }
    
    delete texture;
}

void SDL_UpdateTexture(SDL_Texture* texture, const SDL_Rect* rect,
                        const void* pixels, int pitch) {
    if (!texture || !pixels) return;
    
    // Copy pixel data to texture buffer
    void* textureData = texture->handle;
    if (!textureData) return;
    
    int width = texture->width;
    int height = texture->height;
    
    if (rect) {
        width = rect->w;
        height = rect->h;
        // Calculate offset in texture buffer
        uint8_t* dest = static_cast<uint8_t*>(textureData) + (rect->y * texture->width + rect->x) * 4;
        const uint8_t* src = static_cast<const uint8_t*>(pixels);
        
        for (int y = 0; y < height; ++y) {
            memcpy(dest + y * texture->width * 4, src + y * pitch, width * 4);
        }
    } else {
        memcpy(textureData, pixels, width * height * 4);
    }
}

// Error handling
const char* SDL_GetError() {
    return wiiu::getLastError();
}

void SDL_ClearError() {
    wiiu::clearError();
}

} // namespace SDL

#endif // PLATFORM_WIIU
