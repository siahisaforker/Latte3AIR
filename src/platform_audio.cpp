#include "platform_audio.h"

#if defined(PLATFORM_WIIU) || defined(__WIIU__)

#include "../platform/wiiu/audio.h"
#include <cstring>
#include <algorithm>
#include <cmath>
#include <vector>

// Complete audio implementation using Wii U AX system
namespace platform_audio {

static bool sAudioInitialized = false;
static SDL_AudioSpec sCurrentSpec;
static uint32_t sSampleRate = 44100;
static uint8_t sChannels = 2;
static uint16_t sSamples = 2048;
static float sMusicVolume = 0.8f;
static float sSFXVolume = 0.9f;

// Audio buffer management
class AudioBuffer {
private:
    void* mBuffer;
    size_t mSize;
    size_t mCapacity;
    
public:
    AudioBuffer(size_t initialCapacity = 4096) : mSize(0), mCapacity(initialCapacity) {
        mBuffer = malloc(initialCapacity);
    }
    
    ~AudioBuffer() {
        if (mBuffer) {
            free(mBuffer);
        }
    }
    
    bool reserve(size_t newCapacity) {
        if (newCapacity > mCapacity) {
            void* newBuffer = realloc(mBuffer, newCapacity);
            if (!newBuffer) return false;
            
            mBuffer = newBuffer;
            mCapacity = newCapacity;
        }
        return true;
    }
    
    void append(const void* data, size_t size) {
        if (mSize + size > mCapacity) {
            size_t newCapacity = std::max(mCapacity * 2, mSize + size);
            if (!reserve(newCapacity)) return;
        }
        
        memcpy(static_cast<uint8_t*>(mBuffer) + mSize, data, size);
        mSize += size;
    }
    
    void clear() {
        mSize = 0;
    }
    
    void* getData() const { return mBuffer; }
    size_t getSize() const { return mSize; }
    size_t getCapacity() const { return mCapacity; }
};

// Audio stream for continuous playback
class AudioStream {
private:
    AudioBuffer mBuffer;
    bool mLooping;
    size_t mReadPos;
    size_t mWritePos;
    
public:
    AudioStream(bool looping = false) : mLooping(looping), mReadPos(0), mWritePos(0) {}
    
    void write(const void* data, size_t size) {
        mBuffer.append(data, size);
        mWritePos += size;
    }
    
    size_t read(void* buffer, size_t size) {
        if (mReadPos + size > mBuffer.getSize()) {
            if (!mLooping) {
                return 0; // End of stream
            }
            
            // Handle looping
            size_t available = mBuffer.getSize() - mReadPos;
            if (available < size) {
                // Copy remaining data
                memcpy(buffer, static_cast<uint8_t*>(mBuffer.getData()) + mReadPos, available);
                size_t remaining = size - available;
                
                // Copy from beginning
                if (remaining <= mBuffer.getSize()) {
                    memcpy(static_cast<uint8_t*>(buffer) + available, mBuffer.getData(), remaining);
                    mReadPos = remaining;
                } else {
                    return available; // Not enough data for full read
                }
            } else {
                memcpy(buffer, static_cast<uint8_t*>(mBuffer.getData()) + mReadPos, size);
                mReadPos += size;
            }
        } else {
            memcpy(buffer, static_cast<uint8_t*>(mBuffer.getData()) + mReadPos, size);
            mReadPos += size;
        }
        
        return size;
    }
    
    void reset() {
        mBuffer.clear();
        mReadPos = 0;
        mWritePos = 0;
    }
    
    void* getData() const { return mBuffer.getData(); }
    size_t getSize() const { return mBuffer.getSize(); }
    
    bool isEmpty() const {
        return mBuffer.getSize() == 0;
    }
};

// Global audio streams
static AudioStream sMusicStream(true);  // Music loops
static AudioStream sSFXStream(false);  // SFX doesn't loop

int openAudio(SDL_AudioSpec* desired, SDL_AudioSpec* obtained) {
    if (!desired || !obtained) return -1;
    
    if (!wiiu::initializeAudio()) {
        return -1;
    }
    
    // Configure audio parameters
    sSampleRate = desired->freq;
    sChannels = desired->channels;
    sSamples = desired->samples;
    
    // Validate parameters
    if (sSampleRate <= 0) sSampleRate = 44100;
    if (sChannels <= 0 || sChannels > 2) sChannels = 2;
    if (sSamples <= 0) sSamples = 2048;
    
    // Copy to current spec
    sCurrentSpec.freq = sSampleRate;
    sCurrentSpec.format = AUDIO_S16LSB;
    sCurrentSpec.channels = sChannels;
    sCurrentSpec.samples = sSamples;
    sCurrentSpec.silence = 0;
    sCurrentSpec.userdata = nullptr;
    
    *obtained = sCurrentSpec;
    
    sAudioInitialized = true;
    return 0;
}

void pauseAudio(int pause_on) {
    if (!sAudioInitialized) return;
    
    if (pause_on) {
        wiiu::stopAllSounds();
    }
    
    // Note: AX audio system doesn't have a direct pause/resume
    // We handle this by stopping/starting playback
}

void closeAudio() {
    if (!sAudioInitialized) return;
    
    wiiu::shutdownAudio();
    sAudioInitialized = false;
    
    // Clear streams
    sMusicStream.reset();
    sSFXStream.reset();
}

// Audio playback functions
void playSound(const void* data, int size) {
    if (!sAudioInitialized || !data || size <= 0) return;
    
    // Apply volume scaling
    int16_t* samples = static_cast<int16_t*>(const_cast<void*>(data));
    int sampleCount = size / sizeof(int16_t);
    
    // Create volume-adjusted buffer
    std::vector<int16_t> adjustedSamples(sampleCount);
    for (int i = 0; i < sampleCount; ++i) {
        adjustedSamples[i] = static_cast<int16_t>(samples[i] * sSFXVolume);
    }
    
    // Play through Wii U audio system
    wiiu::playSound(adjustedSamples.data(), adjustedSamples.size() * sizeof(int16_t));
}

void playMusic(const void* data, int size) {
    if (!sAudioInitialized || !data || size <= 0) return;
    
    // Clear previous music
    sMusicStream.reset();
    
    // Apply volume scaling
    int16_t* samples = static_cast<int16_t*>(const_cast<void*>(data));
    int sampleCount = size / sizeof(int16_t);
    
    // Create volume-adjusted buffer
    std::vector<int16_t> adjustedSamples(sampleCount);
    for (int i = 0; i < sampleCount; ++i) {
        adjustedSamples[i] = static_cast<int16_t>(samples[i] * sMusicVolume);
    }
    
    // Write to music stream
    sMusicStream.write(adjustedSamples.data(), adjustedSamples.size() * sizeof(int16_t));
    
    // Start music playback
    wiiu::playMusic(sMusicStream.getData(), sMusicStream.getSize());
}

void stopMusic() {
    if (!sAudioInitialized) return;
    
    wiiu::stopMusic();
    sMusicStream.reset();
}

void stopAllSounds() {
    if (!sAudioInitialized) return;
    
    wiiu::stopAllSounds();
    sSFXStream.reset();
}

// Volume control
void setMusicVolume(float volume) {
    sMusicVolume = std::max(0.0f, std::min(1.0f, volume));
    wiiu::setMusicVolume(sMusicVolume);
}

void setSFXVolume(float volume) {
    sSFXVolume = std::max(0.0f, std::min(1.0f, volume));
    wiiu::setSFXVolume(sSFXVolume);
}

// Format conversion
Uint16 audioFormatToSDL(wiiu::AudioFormat format) {
    switch (format) {
        case wiiu::AUDIO_FORMAT_U8:
            return AUDIO_U8;
        case wiiu::AUDIO_FORMAT_S16LSB:
            return AUDIO_S16LSB;
        case wiiu::AUDIO_FORMAT_S16MSB:
            return AUDIO_S16MSB;
        default:
            return AUDIO_S16LSB;
    }
}

wiiu::AudioFormat sdlFormatToAudio(Uint16 format) {
    switch (format) {
        case AUDIO_U8:
            return wiiu::AUDIO_FORMAT_U8;
        case AUDIO_S16LSB:
            return wiiu::AUDIO_FORMAT_S16LSB;
        case AUDIO_S16MSB:
            return wiiu::AUDIO_FORMAT_S16MSB;
        default:
            return wiiu::AUDIO_FORMAT_S16LSB;
    }
}

// Audio processing utilities
void convertToMono(const int16_t* stereoData, int16_t* monoData, int samples) {
    for (int i = 0; i < samples; ++i) {
        monoData[i] = (stereoData[i * 2] + stereoData[i * 2 + 1]) / 2;
    }
}

void convertToStereo(const int16_t* monoData, int16_t* stereoData, int samples) {
    for (int i = 0; i < samples; ++i) {
        stereoData[i * 2] = monoData[i];
        stereoData[i * 2 + 1] = monoData[i];
    }
}

void applyFadeIn(int16_t* samples, int sampleCount, float fadeTime) {
    int fadeSamples = static_cast<int>(sampleCount * fadeTime);
    for (int i = 0; i < fadeSamples && i < sampleCount; ++i) {
        float multiplier = static_cast<float>(i) / static_cast<float>(fadeSamples);
        samples[i] = static_cast<int16_t>(samples[i] * multiplier);
    }
}

void applyFadeOut(int16_t* samples, int sampleCount, float fadeTime) {
    int fadeSamples = static_cast<int>(sampleCount * fadeTime);
    int startFade = sampleCount - fadeSamples;
    
    for (int i = startFade; i < sampleCount; ++i) {
        float multiplier = static_cast<float>(sampleCount - i) / static_cast<float>(fadeSamples);
        samples[i] = static_cast<int16_t>(samples[i] * multiplier);
    }
}

} // namespace platform_audio

#endif // PLATFORM_WIIU
