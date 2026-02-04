#include "WiiUAudio.h"
#include <coreinit/memory.h>
#include <coreinit/systeminfo.h>
#include <malloc.h>
#include <cstring>

namespace rmx {

WiiUAudioBackend::WiiUAudioBackend() :
    mVoice(nullptr),
    mInitialized(false),
    mPlaying(false),
    mVolume(1.0f),
    mSampleRate(44100),
    mChannels(2),
    mBufferSize(2048)  // Increased buffer size for better latency
{
    mBuffer.data = nullptr;
    mBuffer.size = 0;
    mBuffer.capacity = 0;
    mBuffer.ready = false;
}

WiiUAudioBackend::~WiiUAudioBackend()
{
    shutdown();
}

bool WiiUAudioBackend::initialize(int sampleRate, int channels, int bufferSize)
{
    std::lock_guard<std::mutex> lock(mMutex);
    
    if (mInitialized)
        return true;

    mSampleRate = sampleRate;
    mChannels = channels;
    mBufferSize = bufferSize;

    // Initialize AX audio system
    if (AXInit() != AX_ERROR_NONE)
    {
        return false;
    }

    // Allocate audio buffer
    if (!allocateBuffer(bufferSize * channels * sizeof(int16_t)))
    {
        AXQuit();
        return false;
    }

    // Setup and configure voice
    setupVoice();

    mInitialized = true;
    return true;
}

void WiiUAudioBackend::shutdown()
{
    std::lock_guard<std::mutex> lock(mMutex);
    
    if (!mInitialized)
        return;

    mPlaying = false;

    if (mVoice)
    {
        AXVoiceStop(mVoice);
        AXFreeVoice(mVoice);
        mVoice = nullptr;
    }

    freeBuffer();
    AXQuit();
    mInitialized = false;
}

void WiiUAudioBackend::outputAudio(const void* buffer, size_t size)
{
    if (!mInitialized || !mVoice || !buffer || size == 0)
        return;

    std::lock_guard<std::mutex> lock(mMutex);

    // Ensure our buffer is large enough
    if (size > mBuffer.capacity)
    {
        if (!allocateBuffer(size))
            return;
    }

    // Copy audio data to our buffer
    memcpy(mBuffer.data, buffer, size);
    mBuffer.size = size;

    // Update voice buffer data
    AXVoiceBegin(mVoice);
    AXVoiceSetBufferData(mVoice, mBuffer.data, static_cast<uint32_t>(size), 0);
    AXVoiceSetOffsets(mVoice, 0, static_cast<uint32_t>(size));
    AXVoiceEnd(mVoice);

    // Start playback if not already playing
    if (!mPlaying)
    {
        AXVoiceStart(mVoice);
        mPlaying = true;
    }
}

void WiiUAudioBackend::setVolume(float volume)
{
    mVolume = volume;
    if (mVoice && mInitialized)
    {
        AXVoiceBegin(mVoice);
        AXVoiceSetVolume(mVoice, volume);
        AXVoiceEnd(mVoice);
    }
}

bool WiiUAudioBackend::allocateBuffer(size_t size)
{
    freeBuffer();

    // Allocate aligned memory for audio buffer
    mBuffer.data = static_cast<uint8_t*>(memalign(0x40, size));
    if (!mBuffer.data)
        return false;

    mBuffer.capacity = size;
    mBuffer.size = 0;
    mBuffer.ready = false;

    // Clear the buffer
    memset(mBuffer.data, 0, size);
    return true;
}

void WiiUAudioBackend::freeBuffer()
{
    if (mBuffer.data)
    {
        free(mBuffer.data);
        mBuffer.data = nullptr;
    }
    mBuffer.capacity = 0;
    mBuffer.size = 0;
    mBuffer.ready = false;
}

void WiiUAudioBackend::setupVoice()
{
    // Acquire a voice for audio output
    mVoice = AXAcquireVoice(0, nullptr, nullptr);
    if (!mVoice)
    {
        return;
    }

    AXVoiceBegin(mVoice);

    // Set voice format
    AXVoiceSetFormat(mVoice, 
                     static_cast<uint32_t>(mSampleRate),
                     static_cast<uint16_t>(mChannels),
                     16); // 16-bit samples

    // Set initial volume
    AXVoiceSetVolume(mVoice, mVolume);

    // Disable looping (we'll handle streaming manually)
    AXVoiceSetLoop(mVoice, 0);

    AXVoiceEnd(mVoice);
}

} // namespace rmx
