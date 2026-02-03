#include "platform_wiiu.h"

Renderer* g_renderer = nullptr;

// ----------------------
// Input
// ----------------------
void pollInput(InputState& state) {
    VPADData vpad;
    VPADRead(0, &vpad, 1, nullptr);

    state.a = vpad.btns & VPAD_BUTTON_A;
    state.b = vpad.btns & VPAD_BUTTON_B;
    state.x = vpad.btns & VPAD_BUTTON_X;
    state.y = vpad.btns & VPAD_BUTTON_Y;
    state.start = vpad.btns & VPAD_BUTTON_PLUS;
    state.select = vpad.btns & VPAD_BUTTON_MINUS;

    state.up = vpad.leftStickY > 0.5f;
    state.down = vpad.leftStickY < -0.5f;
    state.left = vpad.leftStickX < -0.5f;
    state.right = vpad.leftStickX > 0.5f;
}

// ----------------------
// Audio
// ----------------------
bool audio_init() {
    return SND_Init() == 0;
}

void audio_play_sound(const void* data, int size) {
    if (!data || size <= 0) return;
    SNDVoice voice;
    memset(&voice, 0, sizeof(SNDVoice));
    SND_SetVoiceData(&voice, (void*)data, size);
    SND_StartVoice(&voice);
}

void audio_play_music(const void* data, int size) {
    audio_play_sound(data, size); // simple streaming stub
}

// ----------------------
// Filesystem
// ----------------------
const char* ROM_PATH = "/vol/external01/sonic3air/rom/Sonic_Knuckles.bin";
const char* SAVE_PATH = "/vol/external01/sonic3air/save/";
const char* CONFIG_PATH = "/vol/external01/sonic3air/config/";

bool file_exists(const char* path) {
    FILE* f = fopen(path, "rb");
    if (f) { fclose(f); return true; }
    return false;
}

bool ensure_dir_exists(const char* path) {
    struct stat st = {0};
    if (stat(path, &st) == -1) {
        mkdir(path, 0777);
        return true;
    }
    return false;
}

bool load_rom(const char* path, void** buffer, size_t* size) {
    if (!file_exists(path)) return false;
    FILE* f = fopen(path, "rb");
    fseek(f, 0, SEEK_END);
    *size = ftell(f);
    fseek(f, 0, SEEK_SET);
    *buffer = malloc(*size);
    fread(*buffer, 1, *size, f);
    fclose(f);
    return true;
}

// ----------------------
// Threading / Timing
// ----------------------
bool create_thread(Thread& thread, ThreadFunc func, void* arg, size_t stackSize) {
    return OSCreateThread(&thread.osThread, (OSThreadFunction)func, arg, nullptr, stackSize, 30, OS_THREAD_ATTR_DETACHED) == 0;
}

void sleep_ms(u32 ms) {
    OSSleepTicks(ms * (OSGetTickFrequency() / 1000));
}

u64 get_time_ms() {
    return OSGetSystemTick() / (OSGetTickFrequency() / 1000);
}
