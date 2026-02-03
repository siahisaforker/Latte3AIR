#pragma once

#include <cstdint>
#include <atomic>
#include <mutex>

#ifdef __cplusplus
extern "C" {
#endif

// sndcore2 types and functions
typedef struct AXVoice AXVoice;
typedef struct AXVoiceEx AXVoiceEx;

typedef struct {
    void* buffer;
    uint32_t size;
} AXAudioBuffer;

typedef enum {
    AX_ERROR_NONE = 0,
    AX_ERROR_INIT_FAILED = -1,
    AX_ERROR_INVALID_PARAM = -2,
    AX_ERROR_OUT_OF_MEMORY = -3
} AXResult;

// sndcore2 function declarations
AXResult AXInit();
void AXQuit();
AXVoice* AXAcquireVoice(uint32_t priority, void* callback, void* userContext);
void AXFreeVoice(AXVoice* voice);
void AXVoiceBegin(AXVoice* voice);
void AXVoiceEnd(AXVoice* voice);
void AXVoiceSetFormat(AXVoice* voice, uint32_t sampleRate, uint16_t channels, uint16_t bitsPerSample);
void AXVoiceSetVolume(AXVoice* voice, float volume);
void AXVoiceSetLoop(AXVoice* voice, uint32_t loopEnabled);
void AXVoiceSetBufferData(AXVoice* voice, const void* buffer, uint32_t size, uint32_t offset);
void AXVoiceSetOffsets(AXVoice* voice, uint32_t loopOffset, uint32_t endOffset);
void AXVoiceStart(AXVoice* voice);
void AXVoiceStop(AXVoice* voice);
void AXVoiceSetState(AXVoice* voice, uint32_t state);

#ifdef __cplusplus
}
#endif

namespace rmx {

class WiiUAudioBackend
{
public:
    WiiUAudioBackend();
    ~WiiUAudioBackend();

    bool initialize(int sampleRate, int channels, int bufferSize);
    void shutdown();
    
    void outputAudio(const void* buffer, size_t size);
    void setVolume(float volume);
    
    bool isInitialized() const { return mInitialized; }

private:
    struct AudioBuffer {
        uint8_t* data;
        size_t size;
        size_t capacity;
        std::atomic<bool> ready;
    };

    AXVoice* mVoice;
    AudioBuffer mBuffer;
    std::mutex mMutex;
    std::atomic<bool> mInitialized;
    std::atomic<bool> mPlaying;
    float mVolume;
    
    int mSampleRate;
    int mChannels;
    int mBufferSize;
    
    bool allocateBuffer(size_t size);
    void freeBuffer();
    void setupVoice();
};

} // namespace rmx
