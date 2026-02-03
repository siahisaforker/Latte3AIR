#pragma once
#include <vpad/input.h>
#include <sndcore2.h>
#include <gx2.h>
#include <os.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/stat.h>

// ----------------------
// Input
// ----------------------
struct InputState {
    bool up, down, left, right;
    bool a, b, x, y;
    bool start, select;
};

// ----------------------
// Renderer forward
// ----------------------
class Renderer;
extern Renderer* g_renderer;

// ----------------------
// Audio
// ----------------------
bool audio_init();
void audio_play_sound(const void* data, int size);
void audio_play_music(const void* data, int size);

// ----------------------
// Filesystem
// ----------------------
extern const char* ROM_PATH;
extern const char* SAVE_PATH;
extern const char* CONFIG_PATH;

bool file_exists(const char* path);
bool ensure_dir_exists(const char* path);
bool load_rom(const char* path, void** buffer, size_t* size);

// ----------------------
// Threading
// ----------------------
typedef void (*ThreadFunc)(void*);
struct Thread {
    OSThread osThread;
};

bool create_thread(Thread& thread, ThreadFunc func, void* arg, size_t stackSize = 0x4000);
void sleep_ms(u32 ms);
u64 get_time_ms();
