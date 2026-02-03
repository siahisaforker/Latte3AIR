#pragma once

#include <cstdint>

namespace wiiu {

// Audio functions for Wii U
bool initializeAudio();
void shutdownAudio();

// Sound playback
void playSound(const void* data, int size);
void playMusic(const void* data, int size);
void stopMusic();
void stopAllSounds();

// Volume control
void setMusicVolume(float volume);
void setSFXVolume(float volume);

}
