// ax_stubs.cpp — sndcore2-compatible AX audio stubs for Wii U
// These provide link-time definitions for AX functions declared in WiiUAudio.h.
// When the real sndcore2 library is linked, these will be overridden by the
// linker's preference for library symbols over weak object symbols.

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <mutex>
#include <atomic>
#include <algorithm>

// Match the AXVoice type from WiiUAudio.h
typedef struct AXVoice AXVoice;

namespace {
    struct FakeVoice {
        uint32_t sampleRate = 44100;
        uint16_t channels = 2;
        uint16_t bitsPerSample = 16;
        float volume = 1.0f;
        uint32_t loopEnabled = 0;
        bool playing = false;
        const void* bufferData = nullptr;
        uint32_t bufferSize = 0;
        uint32_t loopOffset = 0;
        uint32_t endOffset = 0;
    };

    std::vector<FakeVoice*> g_voices;
    std::mutex g_axMutex;
    std::atomic<bool> g_axInitialized{false};
}

// Use the exact same signatures as WiiUAudio.h
extern "C" {

int AXInit()
{
    std::lock_guard<std::mutex> lock(g_axMutex);
    g_axInitialized = true;
    return 0; // AX_ERROR_NONE
}

void AXQuit()
{
    std::lock_guard<std::mutex> lock(g_axMutex);
    for (auto* v : g_voices) delete v;
    g_voices.clear();
    g_axInitialized = false;
}

AXVoice* AXAcquireVoice(uint32_t /*priority*/, void* /*callback*/, void* /*userContext*/)
{
    std::lock_guard<std::mutex> lock(g_axMutex);
    if (!g_axInitialized) return nullptr;
    auto* v = new FakeVoice();
    g_voices.push_back(v);
    return reinterpret_cast<AXVoice*>(v);
}

void AXFreeVoice(AXVoice* voice)
{
    std::lock_guard<std::mutex> lock(g_axMutex);
    auto* v = reinterpret_cast<FakeVoice*>(voice);
    if (!v) return;
    auto it = std::find(g_voices.begin(), g_voices.end(), v);
    if (it != g_voices.end()) g_voices.erase(it);
    delete v;
}

void AXVoiceBegin(AXVoice* /*voice*/) {}
void AXVoiceEnd(AXVoice* /*voice*/) {}

void AXVoiceSetFormat(AXVoice* voice, uint32_t sampleRate, uint16_t channels, uint16_t bitsPerSample)
{
    auto* v = reinterpret_cast<FakeVoice*>(voice);
    if (!v) return;
    v->sampleRate = sampleRate;
    v->channels = channels;
    v->bitsPerSample = bitsPerSample;
}

void AXVoiceSetVolume(AXVoice* voice, float volume)
{
    auto* v = reinterpret_cast<FakeVoice*>(voice);
    if (v) v->volume = volume;
}

void AXVoiceSetLoop(AXVoice* voice, uint32_t loopEnabled)
{
    auto* v = reinterpret_cast<FakeVoice*>(voice);
    if (v) v->loopEnabled = loopEnabled;
}

void AXVoiceSetBufferData(AXVoice* voice, const void* buffer, uint32_t size, uint32_t /*offset*/)
{
    auto* v = reinterpret_cast<FakeVoice*>(voice);
    if (!v) return;
    v->bufferData = buffer;
    v->bufferSize = size;
}

void AXVoiceSetOffsets(AXVoice* voice, uint32_t loopOffset, uint32_t endOffset)
{
    auto* v = reinterpret_cast<FakeVoice*>(voice);
    if (!v) return;
    v->loopOffset = loopOffset;
    v->endOffset = endOffset;
}

void AXVoiceStart(AXVoice* voice)
{
    auto* v = reinterpret_cast<FakeVoice*>(voice);
    if (v) v->playing = true;
}

void AXVoiceStop(AXVoice* voice)
{
    auto* v = reinterpret_cast<FakeVoice*>(voice);
    if (v) v->playing = false;
}

void AXVoiceSetState(AXVoice* voice, uint32_t state)
{
    auto* v = reinterpret_cast<FakeVoice*>(voice);
    if (v) v->playing = (state == 1);
}

} // extern "C"
