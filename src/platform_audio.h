#pragma once

#if defined(PLATFORM_WIIU) || defined(__WIIU__)

#include <cstdint>
#include "platform_sdl.h"

// Audio format constants
#define AUDIO_U8        0x0008
#define AUDIO_S16LSB     0x8010
#define AUDIO_S16MSB     0x9010

namespace wiiu {

enum AudioFormat {
    AUDIO_FORMAT_U8,
    AUDIO_FORMAT_S16LSB,
    AUDIO_FORMAT_S16MSB
};

} // namespace wiiu

namespace platform_audio {

// Audio functions
int openAudio(SDL_AudioSpec* desired, SDL_AudioSpec* obtained);
void pauseAudio(int pause_on);
void closeAudio();

// Playback functions
void playSound(const void* data, int size);
void playMusic(const void* data, int size);
void stopMusic();
void stopAllSounds();

// Volume control
void setMusicVolume(float volume);
void setSFXVolume(float volume);

// Format conversion
Uint16 audioFormatToSDL(wiiu::AudioFormat format);
wiiu::AudioFormat sdlFormatToAudio(Uint16 format);

} // namespace platform_audio

#endif // PLATFORM_WIIU
