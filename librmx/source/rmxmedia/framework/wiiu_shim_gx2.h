// WHB/GX2 shim - adapt to installed WUT headers or provide safe fallbacks.
#pragma once

// If the WUT WHB gfx header is available, include it and map common helpers.
#if __has_include(<whb/gfx.h>)
# include <whb/gfx.h>
# define WIIU_HAS_WHB_HEADERS 1
#else
# define WIIU_HAS_WHB_HEADERS 0
#endif

#if WIIU_HAS_WHB_HEADERS
// Map older/synonym names used in project to available WHB APIs when possible.
// WHBGfxFreeTexture is present; map DestroyTexture to FreeTexture for compatibility.
// Do not macro-map DestroyTexture to FreeTexture; provide explicit wrappers in
// the implementation to avoid signature conflicts across WUT versions.

// Provide weak stubs for create/update helpers if not provided by WHB.
// These stubs return error codes or are no-ops so code compiles and falls back safely.
# ifndef WHBGfxCreateTexture
extern "C" int WHBGfxCreateTexture(int /*w*/, int /*h*/, int /*fmt*/, const void* /*data*/);
# endif
# ifndef WHBGfxUpdateTexture
extern "C" void WHBGfxUpdateTexture(int /*tex*/, int /*x*/, int /*y*/, int /*w*/, int /*h*/, const void* /*data*/);
# endif

# ifndef WHBGfxBindTexture
extern "C" void WHBGfxBindTexture(int /*handle*/);
# endif
# ifndef WHBGfxUnbindTexture
extern "C" void WHBGfxUnbindTexture();
# endif
# ifndef WHBGfxDrawTexturedTriangle
extern "C" void WHBGfxDrawTexturedTriangle(int /*x0*/, int /*y0*/, float /*u0*/, float /*v0*/, int /*x1*/, int /*y1*/, float /*u1*/, float /*v1*/, int /*x2*/, int /*y2*/, float /*u2*/, float /*v2*/);
# endif
# ifndef WHBGfxDestroyTexture
extern "C" void WHBGfxDestroyTexture(int /*tex*/);
# endif

#else
// Minimal GX2/WHB shim for builds that don't have WUT headers available.

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

// Provide create/update/destroy fallbacks
static inline int WHBGfxCreateTexture(int /*w*/, int /*h*/, int /*fmt*/, const void* /*data*/) { return -1; }
static inline void WHBGfxUpdateTexture(int /*tex*/, int /*x*/, int /*y*/, int /*w*/, int /*h*/, const void* /*data*/) { }
static inline void WHBGfxDestroyTexture(int /*tex*/) { }

#endif

// Basic types (if needed)
typedef unsigned int GX2Surface_t;
