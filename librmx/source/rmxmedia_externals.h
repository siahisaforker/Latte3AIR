/*
*	rmx Library
*	Copyright (C) 2008-2025 by Eukaryot
*
*	Published under the GNU GPLv3 open source software license, see license.txt
*	or https://www.gnu.org/licenses/gpl-3.0.en.html
*/

#pragma once


// Library linking via pragma
#if defined(PLATFORM_WINDOWS) && defined(RMX_LIB)
	#pragma comment(lib, "sdl2main.lib")
	#pragma comment(lib, "sdl2.lib")
	#pragma comment(lib, "winmm.lib")
	#pragma comment(lib, "imm32.lib")
	#pragma comment(lib, "version.lib")
	#pragma comment(lib, "setupapi.lib")
	#pragma comment(lib, "opengl32.lib")
#endif

// This is for some reason needed under Linux
#if defined(__GNUC__) && __GNUC__ >= 4
	#define DECLSPEC __attribute__ ((visibility("default")))
#endif


// SDL
#ifdef PLATFORM_WINDOWS
	// Needed for MSYS2
	#if defined(__GNUC__)
		#include <SDL2/SDL.h>
	#else
		#include <SDL/SDL.h>
	#endif

#elif defined(PLATFORM_LINUX)
	#include <SDL2/SDL.h>

// For Wii U we don't include SDL here — a full set of minimal SDL-like
// shims (audio, threading, events) is provided later in the OpenGL/PLATFORM_WIIU
// block so the rest of the engine can compile without the real SDL headers.
#elif defined(PLATFORM_WIIU)
	/* intentionally empty: Wii U SDL shims are defined below */

#else
	#include <SDL.h>
#endif


// OpenGL
#if defined(PLATFORM_WINDOWS)
	#define ALLOW_LEGACY_OPENGL
	#define RMX_USE_GLEW

#elif defined(PLATFORM_LINUX)
	#if defined(RMX_LINUX_ENFORCE_GLES2)	// Build option: Use OpenGL ES 2
		#define RMX_USE_GLES2
		#define GL_GLEXT_PROTOTYPES
		#include <GLES2/gl2.h>
		#include <GLES2/gl2ext.h>
	#else
		#define RMX_USE_GLEW
	#endif

#elif defined(PLATFORM_MAC)
	#define ALLOW_LEGACY_OPENGL		// Should be removed for macOS I guess?
	#include <OpenGL/gl3.h>
	#include <OpenGL/glu.h>

#elif defined(PLATFORM_WEB)
	#include <GL/glew.h>

#elif defined(PLATFORM_ANDROID)
	#define RMX_USE_GLES2
	#define GL_GLEXT_PROTOTYPES
	#include <GLES2/gl2.h>
	#include <GLES2/gl2ext.h>

#elif defined(PLATFORM_IOS)
	#define RMX_USE_GLES2
	#define GL_GLEXT_PROTOTYPES
	#include <OpenGLES/ES2/gl.h>
	#include <OpenGLES/ES2/glext.h>

#elif defined(PLATFORM_SWITCH)
	#include <EGL/egl.h>    // EGL library
	#include <EGL/eglext.h> // EGL extensions
	#include <glad/glad.h>  // glad library (OpenGL loader)
	#define RMX_USE_GLAD
	#define GL_LUMINANCE GL_RED

#elif defined(PLATFORM_VITA)
	#include <vitaGL.h>
	#define RMX_USE_GLES2

#elif defined(PLATFORM_WIIU)
	// Wii U (WUT/WHB) - provide minimal SDL-like types and lightweight
	// thread/mutex/cond/event shims so the engine compiles without SDL.
	// These are compile-time placeholders mapping onto the C++ std::thread/
	// mutex primitives; they are not a full SDL implementation.
	#include <cstdint>
	#include <thread>
	#include <mutex>
	#include <condition_variable>
	#include <functional>

	typedef uint32_t SDL_AudioDeviceID;

	struct SDL_AudioSpec
	{
		int freq;
		uint16_t format;
		uint8_t channels;
		uint16_t samples;
		void (*callback)(void* userdata, uint8_t* stream, int len);
		void* userdata;
	};

	// Minimal SDL threading primitives (wrappers using std::*)
	struct SDL_mutex { std::mutex m; };
	inline SDL_mutex* SDL_CreateMutex() { return new SDL_mutex(); }
	inline void SDL_DestroyMutex(SDL_mutex* m) { delete m; }
	inline int SDL_LockMutex(SDL_mutex* m) { m->m.lock(); return 0; }
	inline int SDL_UnlockMutex(SDL_mutex* m) { m->m.unlock(); return 0; }
	inline int SDL_TryLockMutex(SDL_mutex* m) { return m->m.try_lock() ? 0 : 1; }

	struct SDL_cond { std::condition_variable cv; };
	inline SDL_cond* SDL_CreateCond() { return new SDL_cond(); }
	inline void SDL_DestroyCond(SDL_cond* c) { delete c; }
	inline int SDL_CondSignal(SDL_cond* c) { c->cv.notify_one(); return 0; }
	inline int SDL_CondBroadcast(SDL_cond* c) { c->cv.notify_all(); return 0; }
	inline int SDL_CondWait(SDL_cond* c, SDL_mutex* m) { std::unique_lock<std::mutex> lk(m->m); c->cv.wait(lk); return 0; }
	inline int SDL_CondWaitTimeout(SDL_cond* c, SDL_mutex* m, uint32_t ms) { std::unique_lock<std::mutex> lk(m->m); std::cv_status st = c->cv.wait_for(lk, std::chrono::milliseconds(ms)); return (st==std::cv_status::timeout) ? 1 : 0; }

	// Minimal SDL thread wrapper
	struct SDL_Thread { std::thread t; int retval = 0; };
	using SDL_ThreadFunction = int(*)(void*);
	inline SDL_Thread* SDL_CreateThread(SDL_ThreadFunction func, const char* name, void* data)
	{
		SDL_Thread* th = new SDL_Thread();
		th->t = std::thread([th,func,data](){ th->retval = func(data); });
		return th;
	}
	inline SDL_Thread* SDL_CreateThreadWithStackSize(SDL_ThreadFunction func, const char* name, size_t /*stack*/, void* data)
	{
		return SDL_CreateThread(func, name, data);
	}
	inline void SDL_DetachThread(SDL_Thread* th) { th->t.detach(); delete th; }
	inline void SDL_WaitThread(SDL_Thread* th, int* out) { if (th->t.joinable()) th->t.join(); if (out) *out = th->retval; delete th; }

	// Minimal SDL event and helper definitions so the engine compiles without
	// linking the real SDL headers. Full event handling is implemented via
	// the platform input feeder and the runtime should not rely on these
	// stub implementations for actual input delivery.

	typedef uint32_t Uint32;
	typedef uint8_t Uint8;

	// SDL init flags
	#define SDL_INIT_VIDEO 0x00000020

	// Basic SDL functions (no-op/stub implementations)
	inline int SDL_Init(Uint32 /*flags*/) { return 0; }
	inline void SDL_Quit() {}
	inline const char* SDL_GetError() { return ""; }

	// Timing helpers declared earlier (SDL_GetTicks / SDL_Delay) are provided below

	// SDL window/event/button/key structures (minimal fields used by the engine)
	struct SDL_Keysym { int sym; int scancode; int mod; };
	struct SDL_KeyboardEvent { Uint32 type; SDL_Keysym keysym; int repeat; };
	struct SDL_TextInputEvent { Uint32 type; char text[32]; };
	struct SDL_MouseButtonEvent { Uint32 type; int button; int x; int y; };
	struct SDL_MouseWheelEvent { Uint32 type; int y; };
	struct SDL_MouseMotionEvent { Uint32 type; int x; int y; };
	struct SDL_WindowEvent { Uint32 type; int event; int data1; int data2; };

	union SDL_Event
	{
		Uint32 type;
		SDL_WindowEvent window;
		SDL_KeyboardEvent key;
		SDL_TextInputEvent text;
		SDL_MouseButtonEvent button;
		SDL_MouseWheelEvent wheel;
		SDL_MouseMotionEvent motion;
	};

	// Event constants used by the engine
	#define SDL_QUIT 0x100
	#define SDL_WINDOWEVENT 0x200
	#define SDL_WINDOWEVENT_RESIZED 0x01
	#define SDL_WINDOWEVENT_SIZE_CHANGED 0x02
	#define SDL_KEYDOWN 0x300
	#define SDL_KEYUP 0x301
	#define SDL_TEXTINPUT 0x400
	#define SDL_MOUSEBUTTONDOWN 0x500
	#define SDL_MOUSEBUTTONUP 0x501
	#define SDL_MOUSEWHEEL 0x600
	#define SDL_MOUSEMOTION 0x601

	// Number of scancodes the engine expects (used in static_asserts)
	#define SDL_NUM_SCANCODES 0x0200

	// Minimal SDL RWops implementation backed by std C FILE* so file IO
	// functions that rely on SDL_RW* can work on the Wii U build.
	typedef int64_t Sint64;

	struct SDL_RWops { FILE* fp; };

	inline SDL_RWops* SDL_RWFromFile(const char* file, const char* mode)
	{
		FILE* fp = fopen(file, mode);
		if (!fp) return nullptr;
		SDL_RWops* ops = new SDL_RWops(); ops->fp = fp; return ops;
	}

	inline void SDL_RWclose(SDL_RWops* ctx)
	{
		if (!ctx) return;
		if (ctx->fp) fclose(ctx->fp);
		delete ctx;
	}

	inline Sint64 SDL_RWsize(SDL_RWops* ctx)
	{
		if (!ctx || !ctx->fp) return -1;
		long cur = ftell(ctx->fp);
		fseek(ctx->fp, 0, SEEK_END);
		long sz = ftell(ctx->fp);
		fseek(ctx->fp, cur, SEEK_SET);
		return (Sint64)sz;
	}

	inline Sint64 SDL_RWtell(SDL_RWops* ctx)
	{
		if (!ctx || !ctx->fp) return -1;
		return (Sint64)ftell(ctx->fp);
	}

	inline size_t SDL_RWread(SDL_RWops* ctx, void* ptr, size_t size, size_t maxnum)
	{
		if (!ctx || !ctx->fp) return 0;
		return fread(ptr, size, maxnum, ctx->fp);
	}

	inline Sint64 SDL_RWseek(SDL_RWops* ctx, Sint64 offset, int whence)
	{
		if (!ctx || !ctx->fp) return -1;
		if (fseek(ctx->fp, (long)offset, whence) != 0) return -1;
		return (Sint64)ftell(ctx->fp);
	}

	#define RW_SEEK_SET SEEK_SET

	// Polling stub: no events by default. Platform input should be provided
	// via the InputFeeder implementation instead.
	inline int SDL_PollEvent(SDL_Event* /*ev*/) { return 0; }
	inline int SDL_WarpMouseInWindow(void* /*win*/, int /*x*/, int /*y*/) { return 0; }

	#define SDLK_SCANCODE_MASK 0x40000000

	// Minimal GL type aliases and constants for compilation compatibility
	typedef unsigned int GLuint;
	typedef unsigned int GLenum;
	typedef int GLint;
	typedef unsigned int GLbitfield;

	// Basic float type used by GL APIs
	typedef float GLfloat;

	// Minimal GL function stubs so GLTools and other headers can compile
	// These are no-ops for now and should be replaced by real GX2 implementations
	inline void glEnable(GLenum) {}
	inline void glDisable(GLenum) {}
	inline void glViewport(GLint /*x*/, GLint /*y*/, GLint /*w*/, GLint /*h*/) {}
	inline GLenum glGetError() { return 0; }

	#ifndef GL_NO_ERROR
		#define GL_NO_ERROR 0
	#endif

	// Additional GL typedefs/constants used by engine
	typedef int GLsizei;
	typedef ptrdiff_t GLsizeiptr;
	typedef unsigned char GLboolean;

	#define GL_NONE 0
	#define GL_INVALID_OPERATION 0x0502
	#define GL_INVALID_ENUM 0x0500
	#define GL_INVALID_VALUE 0x0501
	#define GL_OUT_OF_MEMORY 0x0505
	#define GL_INVALID_FRAMEBUFFER_OPERATION 0x0506
	#define GL_FLOAT 0x1406
	#define GL_FALSE 0
	#define GL_TRUE 1
	#define GL_ARRAY_BUFFER 0x8892
	#define GL_STATIC_DRAW 0x88E4

	// Minimal GL buffer/VAO/vertex API stubs (no-op)
	inline void glGenBuffers(GLsizei n, unsigned int* ids) { for (GLsizei i=0;i<n;++i) ids[i]=0; }
	inline void glDeleteBuffers(GLsizei n, const unsigned int* ids) { (void)n; (void)ids; }
	inline void glBindBuffer(GLenum /*target*/, unsigned int /*buffer*/) {}
	inline void glBufferData(GLenum /*target*/, GLsizeiptr /*size*/, const void* /*data*/, GLenum /*usage*/) {}

	inline void glGenVertexArrays(GLsizei n, unsigned int* ids) { for (GLsizei i=0;i<n;++i) ids[i]=0; }
	inline void glDeleteVertexArrays(GLsizei n, const unsigned int* ids) { (void)n; (void)ids; }
	inline void glBindVertexArray(unsigned int /*array*/) {}

	inline void glEnableVertexAttribArray(unsigned int /*index*/) {}
	inline void glDisableVertexAttribArray(unsigned int /*index*/) {}
	inline void glVertexAttribPointer(unsigned int /*index*/, int /*size*/, unsigned int /*type*/, GLboolean /*normalized*/, GLsizei /*stride*/, const void* /*pointer*/) {}
	inline void glDrawArrays(unsigned int /*mode*/, int /*first*/, GLsizei /*count*/) {}

	// Additional GL stubs used by Shader/Texture so the code compiles on Wii U

	typedef char GLchar;

	#ifndef GL_TEXTURE0
	#define GL_TEXTURE0 0x84C0
	#endif

	inline void glUseProgram(GLuint) {}
	inline void glActiveTexture(GLenum) {}
	inline void glDeleteProgram(GLuint) {}
	inline void glDeleteShader(GLuint) {}
	inline GLuint glCreateShader(GLenum) { return 0; }
	inline void glShaderSource(GLuint, GLsizei, const GLchar* const*, const GLint*) {}
	inline void glCompileShader(GLuint) {}
	inline void glGetShaderiv(GLuint, GLenum, GLint* p) { if (p) *p = 0; }
	inline void glGetShaderInfoLog(GLuint, GLsizei, GLsizei*, GLchar*) {}
	inline GLuint glCreateProgram() { return 0; }
	inline void glAttachShader(GLuint, GLuint) {}
	inline void glLinkProgram(GLuint) {}
	inline void glGetProgramiv(GLuint, GLenum, GLint* p) { if (p) *p = 0; }
	inline void glGetProgramInfoLog(GLuint, GLsizei, GLsizei*, GLchar*) {}
	inline GLint glGetUniformLocation(GLuint, const char*) { return -1; }
	inline GLint glGetAttribLocation(GLuint, const char*) { return -1; }

	inline void glUniform1i(GLint, GLint) {}
	inline void glUniform1f(GLint, GLfloat) {}
	inline void glUniform2iv(GLint, GLsizei, const GLint*) {}
	inline void glUniform3iv(GLint, GLsizei, const GLint*) {}
	inline void glUniform4iv(GLint, GLsizei, const GLint*) {}
	inline void glUniform2fv(GLint, GLsizei, const GLfloat*) {}
	inline void glUniform3fv(GLint, GLsizei, const GLfloat*) {}
	inline void glUniform4fv(GLint, GLsizei, const GLfloat*) {}
	inline void glUniformMatrix3fv(GLint, GLsizei, GLboolean, const GLfloat*) {}
	inline void glUniformMatrix4fv(GLint, GLsizei, GLboolean, const GLfloat*) {}
	inline void glBindAttribLocation(GLuint, GLuint, const char*) {}

	inline void glGenTextures(GLsizei n, unsigned int* ids) { for (GLsizei i=0;i<n;++i) ids[i]=0; }
	inline void glDeleteTextures(GLsizei n, const unsigned int* ids) { (void)n; (void)ids; }
	inline void glBindTexture(GLenum, GLuint) {}
	inline void glTexImage2D(GLenum, GLint, GLint, GLsizei, GLsizei, GLint, GLenum, GLenum, const void*) {}
	inline void glTexSubImage2D(GLenum, GLint, GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, const void*) {}
	inline void glCopyTexImage2D(GLenum, GLint, GLenum, GLint, GLint, GLsizei, GLsizei, GLint) {}
	inline void glGenerateMipmap(GLenum) {}
	inline void glTexParameteri(GLenum, GLenum, GLint) {}

	// Framebuffer/renderbuffer & misc helpers
	inline GLboolean glIsRenderbuffer(GLuint) { return GL_FALSE; }
	inline void glGenRenderbuffers(GLsizei n, GLuint* ids) { for (GLsizei i=0;i<n;++i) ids[i]=0; }
	inline void glBindRenderbuffer(GLenum, GLuint) {}
	inline void glRenderbufferStorage(GLenum, GLenum, GLsizei, GLsizei) {}
	inline void glDeleteRenderbuffers(GLsizei n, const GLuint* ids) { (void)n; (void)ids; }

	inline void glGenFramebuffers(GLsizei n, GLuint* ids) { for (GLsizei i=0;i<n;++i) ids[i]=0; }
	inline void glDeleteFramebuffers(GLsizei n, const GLuint* ids) { (void)n; (void)ids; }
	inline void glBindFramebuffer(GLenum, GLuint) {}
	inline GLenum glCheckFramebufferStatus(GLenum) { return 0; }
	inline void glFramebufferTexture2D(GLenum, GLenum, GLenum, GLuint, GLint) {}
	inline void glFramebufferRenderbuffer(GLenum, GLenum, GLenum, GLuint) {}
	inline GLboolean glIsFramebuffer(GLuint) { return GL_FALSE; }

	inline void glClear(GLbitfield) {}
	inline void glClearColor(GLfloat, GLfloat, GLfloat, GLfloat) {}
	inline void glBlendFunc(GLenum, GLenum) {}

	// Texture/render helper
	inline GLboolean glIsTexture(GLuint) { return GL_FALSE; }


	// Common GL enums
	#ifndef GL_COMPILE_STATUS
	#define GL_COMPILE_STATUS 0x8B81
	#endif
	#ifndef GL_INFO_LOG_LENGTH
	#define GL_INFO_LOG_LENGTH 0x8B84
	#endif
	#ifndef GL_LINK_STATUS
	#define GL_LINK_STATUS 0x8B82
	#endif
	#ifndef GL_ONE
	#define GL_ONE 1
	#endif
	#ifndef GL_ZERO
	#define GL_ZERO 0
	#endif
	#ifndef GL_SRC_ALPHA
	#define GL_SRC_ALPHA 0x0302
	#endif
	#ifndef GL_ONE_MINUS_SRC_ALPHA
	#define GL_ONE_MINUS_SRC_ALPHA 0x0303
	#endif
	#ifndef GL_NEAREST_MIPMAP_NEAREST
	#define GL_NEAREST_MIPMAP_NEAREST 0x2702
	#endif
	#ifndef GL_LINEAR_MIPMAP_LINEAR
	#define GL_LINEAR_MIPMAP_LINEAR 0x2703
	#endif
	#ifndef GL_TEXTURE_CUBE_MAP
	#define GL_TEXTURE_CUBE_MAP 0x8513
	#endif
	#ifndef GL_TEXTURE_CUBE_MAP_POSITIVE_X
	#define GL_TEXTURE_CUBE_MAP_POSITIVE_X 0x8515
	#endif

	#ifndef GL_VERTEX_SHADER
	#define GL_VERTEX_SHADER 0x8B31
	#endif
	#ifndef GL_FRAGMENT_SHADER
	#define GL_FRAGMENT_SHADER 0x8B30
	#endif

	#ifndef GL_RENDERBUFFER
	#define GL_RENDERBUFFER 0x8D41
	#endif
	#ifndef GL_FRAMEBUFFER
	#define GL_FRAMEBUFFER 0x8D40
	#endif
	#ifndef GL_FRAMEBUFFER_COMPLETE
	#define GL_FRAMEBUFFER_COMPLETE 0x8CD5
	#endif
	#ifndef GL_TEXTURE_MIN_FILTER
	#define GL_TEXTURE_MIN_FILTER 0x2801
	#endif
	#ifndef GL_TEXTURE_MAG_FILTER
	#define GL_TEXTURE_MAG_FILTER 0x2800
	#endif
	#ifndef GL_TEXTURE_WRAP_S
	#define GL_TEXTURE_WRAP_S 0x2802
	#endif
	#ifndef GL_TEXTURE_WRAP_T
	#define GL_TEXTURE_WRAP_T 0x2803
	#endif
	#ifndef GL_MIRRORED_REPEAT
	#define GL_MIRRORED_REPEAT 0x8370
	#endif




	// Basic timing helpers used around the engine
	inline void SDL_Delay(uint32_t ms) { std::this_thread::sleep_for(std::chrono::milliseconds(ms)); }
	inline uint32_t SDL_GetTicks()
	{
		using namespace std::chrono;
		auto now = steady_clock::now().time_since_epoch();
		return static_cast<uint32_t>(duration_cast<milliseconds>(now).count());
	}

	#define GL_TEXTURE_2D 0x0DE1
	#define GL_RGBA 0x1908
	#define GL_RGB 0x1907
	#define GL_UNSIGNED_BYTE 0x1401
	#define GL_LINEAR 0x2601
	#define GL_NEAREST 0x2600
	#define GL_REPEAT 0x2901
	#define GL_CLAMP_TO_EDGE 0x812F
	#define GL_RGB8 GL_RGB
	#define GL_RGBA8 GL_RGBA
	#define GL_DEPTH_COMPONENT 0x1902


#else
	#error Unsupported platform
#endif


#if defined(RMX_USE_GLES2) && !defined(__EMSCRIPTEN__)
	#if !defined(PLATFORM_LINUX) && !defined(__vita__)
		#define GL_RGB8				 GL_RGB
		#define GL_RGBA8			 GL_RGBA
		#define glGenVertexArrays	 glGenVertexArraysOES
		#define glDeleteVertexArrays glDeleteVertexArraysOES
		#define glBindVertexArray	 glBindVertexArrayOES
	#endif
	#define glClearDepth glClearDepthf
	#define glDepthRange glDepthRangef
#endif


#ifdef RMX_USE_GLEW
	#ifndef GLEW_STATIC
		#define GLEW_STATIC
	#endif
	#define GLEW_NO_GLU
	#include "rmxmedia/_glew/GL/glew.h"
#endif
