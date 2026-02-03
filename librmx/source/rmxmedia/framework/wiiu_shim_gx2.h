// Minimal GX2/WHB shim for builds that don't have WUT headers available.
#pragma once

// GX2 minimal stubs
inline void GX2Init() {}
inline void GX2Flush() {}
inline void GX2SetClearColor(unsigned int /*color*/) {}
inline void GX2Clear(int /*mask*/) {}
#define GX2_CLEAR_ALL 0xFFFFFFFF

// WHB minimal stubs
inline void WHBProcInitialize() {}
inline int WHBGfxInit() { return 0; }
inline void WHBGfxTerm() {}
inline void WHBProcShutdown() {}
inline void WHBGfxSwapBuffers() {}

// WHB helper draw stubs used by Painter_WiiU
inline void WHBGfxDrawQuad(int /*x*/, int /*y*/, int /*w*/, int /*h*/, unsigned int /*color*/) {}
inline void WHBGfxBindTexture(int /*handle*/) {}
inline void WHBGfxDrawTexturedQuad(int /*x*/, int /*y*/, int /*w*/, int /*h*/, unsigned int /*color*/) {}
inline void WHBGfxUnbindTexture() {}
inline void WHBGfxEnableScissor(int /*x*/, int /*y*/, int /*w*/, int /*h*/) {}
inline void WHBGfxDisableScissor() {}
inline void WHBGfxDrawTexturedTriangle(int /*x0*/, int /*y0*/, float /*u0*/, float /*v0*/, int /*x1*/, int /*y1*/, float /*u1*/, float /*v1*/, int /*x2*/, int /*y2*/, float /*u2*/, float /*v2*/) {}

// Basic types (if needed)
typedef unsigned int GX2Surface_t;
