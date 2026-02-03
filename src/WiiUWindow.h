#pragma once
#include <cstdint>
#include <cstddef>

class WiiUWindow {
public:
    bool initialize(int srcWidth, int srcHeight);
    void shutdown();
    void present(const uint32_t* pixels, int srcWidth, int srcHeight);
    bool isInitialized() const { return mInitialized; }

private:
    void blitScale(const uint32_t* src, int sw, int sh,
                   uint32_t* dst, int dw, int dh, int dstStridePixels);

    void* mTVBuffer = nullptr;
    void* mDRCBuffer = nullptr;
    std::size_t mTVSize = 0;
    std::size_t mDRCSize = 0;
    int mTVWidth = 0;
    int mTVHeight = 0;
    int mDRCWidth = 0;
    int mDRCHeight = 0;
    int mTVStridePixels = 0;
    int mDRCStridePixels = 0;
    bool mInitialized = false;
};
