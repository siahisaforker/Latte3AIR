// Minimal Wii U stub for WavLoader
#include "rmxmedia/audiovideo/AudioManager.h"

namespace rmx
{
    bool WavLoader::load(AudioBuffer* buffer, const String& source, const String& params)
    {
        (void)buffer;
        (void)source;
        (void)params;
        return false; // Not implemented on Wii U stub
    }
}
