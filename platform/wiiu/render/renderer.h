#pragma once

/**
 * Wii U rendering contract.
 * Single interface the engine talks to. No SDL types. No engine leakage.
 * Framebuffer format: RGBA8888, little-endian CPU buffer, explicit stride (pitch).
 */
namespace wiiu {

class Renderer {
public:
    virtual ~Renderer() = default;

    virtual bool init() = 0;
    virtual void shutdown() = 0;

    virtual void beginFrame() = 0;
    virtual void uploadFrameBuffer(
        const void* pixels,
        int width,
        int height,
        int pitch
    ) = 0;
    virtual void present() = 0;
};

}
