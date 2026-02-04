#include <cstdint>
#include <cstdlib>

extern "C" {
    int AXInit() { return 0; }
    void AXQuit() {}
    void* AXAcquireVoice(int id, void* a, void* b) { (void)id; (void)a; (void)b; return nullptr; }
    void AXFreeVoice(void* v) { (void)v; }
    void AXVoiceBegin(void* v) { (void)v; }
    void AXVoiceEnd(void* v) { (void)v; }
    void AXVoiceSetFormat(void* v, uint32_t freq, uint16_t channels, int bits) { (void)v; (void)freq; (void)channels; (void)bits; }
    void AXVoiceSetVolume(void* v, float vol) { (void)v; (void)vol; }
    void AXVoiceSetLoop(void* v, int loop) { (void)v; (void)loop; }
    void AXVoiceStop(void* v) { (void)v; }
    void AXVoiceSetBufferData(void* v, const void* data, uint32_t size, uint32_t param) { (void)v; (void)data; (void)size; (void)param; }
    void AXVoiceSetOffsets(void* v, uint32_t a, uint32_t b) { (void)v; (void)a; (void)b; }
    void AXVoiceStart(void* v) { (void)v; }
}
