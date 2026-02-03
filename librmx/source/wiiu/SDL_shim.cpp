#include "wiiu/SDL_shim.h"
#include "wiiu/WiiUGfx.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <unordered_map>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <atomic>
#include <vector>
#include <chrono>
#include <malloc.h>

#include <vpad/input.h>

struct SDL_RWops
{
	FILE* fp = nullptr;
};

struct SDL_Thread
{
	std::thread thread;
};

struct SDL_mutex
{
	std::mutex mutex;
};

struct SDL_cond
{
	std::condition_variable cond;
};

struct SDL_Joystick
{
	int index = 0;
};

struct SDL_GameController
{
	int index = 0;
};

namespace
{
	constexpr Uint32 kWindowIdBase = 1;
	std::atomic<Uint32> gNextWindowId{ kWindowIdBase };
	std::string gLastError;

	struct AudioDevice
	{
		SDL_AudioSpec spec{};
		std::thread thread;
		std::mutex mutex;
		std::atomic<bool> running{ false };
		std::atomic<bool> paused{ true };
	};

	std::unordered_map<SDL_AudioDeviceID, AudioDevice*> gAudioDevices;
	std::atomic<SDL_AudioDeviceID> gNextAudioId{ 1 };

	VPADStatus readVpad()
	{
		VPADStatus status{};
		VPADReadError error = VPAD_READ_NO_SAMPLES;
		VPADRead(VPAD_CHAN_0, &status, 1, &error);
		if (error != VPAD_READ_SUCCESS)
		{
			std::memset(&status, 0, sizeof(status));
		}
		return status;
	}

	Sint16 toAxis(float value)
	{
		if (value < -1.0f) value = -1.0f;
		if (value > 1.0f) value = 1.0f;
		return static_cast<Sint16>(value * 32767.0f);
	}

	Uint8 dpadToHat(Uint32 hold)
	{
		Uint8 hat = SDL_HAT_CENTERED;
		if (hold & VPAD_BUTTON_UP) hat |= SDL_HAT_UP;
		if (hold & VPAD_BUTTON_DOWN) hat |= SDL_HAT_DOWN;
		if (hold & VPAD_BUTTON_LEFT) hat |= SDL_HAT_LEFT;
		if (hold & VPAD_BUTTON_RIGHT) hat |= SDL_HAT_RIGHT;
		return hat;
	}
}

int SDL_Init(Uint32)
{
	VPADInit();
	return 0;
}

int SDL_InitSubSystem(Uint32)
{
	return 0;
}

void SDL_Quit()
{
}

void SDL_QuitSubSystem(Uint32)
{
}

const char* SDL_GetError()
{
	return gLastError.c_str();
}

Uint32 SDL_GetTicks()
{
	static const auto start = std::chrono::steady_clock::now();
	const auto now = std::chrono::steady_clock::now();
	const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count();
	return static_cast<Uint32>(ms);
}

void SDL_Delay(Uint32 ms)
{
	std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

int SDL_PollEvent(SDL_Event*)
{
	// Event queue is not used on Wii U; input is polled via joystick APIs.
	return 0;
}

SDL_Window* SDL_CreateWindow(const char*, int, int, int w, int h, Uint32)
{
	SDL_Window* window = new SDL_Window();
	window->w = w;
	window->h = h;
	window->id = gNextWindowId.fetch_add(1);
	window->surface = nullptr;

	rmx::WiiUGfx::initialize(w, h);
	return window;
}

void SDL_DestroyWindow(SDL_Window* window)
{
	if (!window)
		return;
	if (window->surface)
	{
		if (window->surface->pixels)
			std::free(window->surface->pixels);
		delete window->surface->format;
		delete window->surface;
		window->surface = nullptr;
	}
	delete window;
	rmx::WiiUGfx::shutdown();
}

SDL_Surface* SDL_GetWindowSurface(SDL_Window* window)
{
	if (!window)
		return nullptr;

	if (!window->surface)
	{
		SDL_Surface* surface = new SDL_Surface();
		surface->w = window->w;
		surface->h = window->h;
		surface->pitch = surface->w * 4;
		surface->format = new SDL_PixelFormat();
		surface->format->format = SDL_PIXELFORMAT_ABGR8888;

		const size_t size = static_cast<size_t>(surface->pitch) * static_cast<size_t>(surface->h);
		surface->pixels = memalign(64, (size + 63) & ~static_cast<size_t>(63));
		if (surface->pixels)
		{
			std::memset(surface->pixels, 0, size);
		}
		window->surface = surface;
	}

	return window->surface;
}

int SDL_UpdateWindowSurface(SDL_Window* window)
{
	if (!window || !window->surface || !window->surface->pixels)
		return -1;
	rmx::WiiUGfx::present(static_cast<const uint32_t*>(window->surface->pixels), window->surface->w, window->surface->h);
	return 0;
}

int SDL_LockSurface(SDL_Surface*)
{
	return 0;
}

void SDL_UnlockSurface(SDL_Surface*)
{
}

SDL_Surface* SDL_CreateRGBSurfaceFrom(void* pixels, int width, int height, int, int pitch, Uint32, Uint32, Uint32, Uint32)
{
	SDL_Surface* surface = new SDL_Surface();
	surface->w = width;
	surface->h = height;
	surface->pitch = pitch;
	surface->pixels = pixels;
	surface->format = new SDL_PixelFormat();
	surface->format->format = SDL_PIXELFORMAT_ABGR8888;
	return surface;
}

void SDL_FreeSurface(SDL_Surface* surface)
{
	if (!surface)
		return;
	delete surface->format;
	delete surface;
}

void SDL_SetWindowIcon(SDL_Window*, SDL_Surface*)
{
}

void SDL_GetWindowSize(SDL_Window* window, int* w, int* h)
{
	if (!window)
		return;
	if (w) *w = window->w;
	if (h) *h = window->h;
}

Uint32 SDL_GetWindowID(SDL_Window* window)
{
	return window ? window->id : 0;
}

int SDL_GetWindowDisplayIndex(SDL_Window*)
{
	return 0;
}

int SDL_SetWindowFullscreen(SDL_Window*, Uint32)
{
	return 0;
}

void SDL_SetWindowSize(SDL_Window* window, int w, int h)
{
	if (!window)
		return;
	window->w = w;
	window->h = h;
	if (window->surface)
	{
		if (window->surface->pixels)
			std::free(window->surface->pixels);
		delete window->surface->format;
		delete window->surface;
		window->surface = nullptr;
	}
}

void SDL_SetWindowPosition(SDL_Window*, int, int)
{
}

void SDL_SetWindowResizable(SDL_Window*, SDL_bool)
{
}

void SDL_SetWindowBordered(SDL_Window*, SDL_bool)
{
}

int SDL_ShowCursor(int)
{
	return 0;
}

int SDL_SetHint(const char*, const char*)
{
	return 1;
}

int SDL_setenv(const char*, const char*, int)
{
	return 0;
}

Uint16 SDL_GetModState()
{
	return KMOD_NONE;
}

void SDL_WarpMouseInWindow(SDL_Window*, int, int)
{
}

int SDL_GetDisplayBounds(int, SDL_Rect* rect)
{
	if (!rect)
		return -1;
	rect->x = 0;
	rect->y = 0;
	rect->w = 1280;
	rect->h = 720;
	return 0;
}

int SDL_GetDesktopDisplayMode(int, SDL_DisplayMode* mode)
{
	if (!mode)
		return -1;
	mode->w = 1280;
	mode->h = 720;
	mode->refresh_rate = 60;
	mode->driverdata = nullptr;
	return 0;
}

SDL_bool SDL_IsScreenSaverEnabled()
{
	return SDL_FALSE;
}

void SDL_EnableScreenSaver()
{
}

void SDL_DisableScreenSaver()
{
}

int SDL_GL_SetAttribute(int, int)
{
	return 0;
}

void* SDL_GL_CreateContext(SDL_Window*)
{
	return nullptr;
}

void SDL_GL_SetSwapInterval(int)
{
}

void SDL_GL_SwapWindow(SDL_Window*)
{
}

void* SDL_GL_GetCurrentContext()
{
	return nullptr;
}

SDL_bool SDL_IsTextInputActive()
{
	return SDL_FALSE;
}

void SDL_StartTextInput()
{
}

void SDL_StopTextInput()
{
}

int SDL_NumJoysticks()
{
	return 1;
}

SDL_Joystick* SDL_JoystickOpen(int device_index)
{
	static SDL_Joystick joystick;
	joystick.index = device_index;
	return &joystick;
}

void SDL_JoystickClose(SDL_Joystick*)
{
}

int SDL_JoystickNumButtons(SDL_Joystick*)
{
	return 16;
}

int SDL_JoystickNumAxes(SDL_Joystick*)
{
	return 4;
}

int SDL_JoystickNumHats(SDL_Joystick*)
{
	return 1;
}

Sint16 SDL_JoystickGetAxis(SDL_Joystick*, int axis)
{
	const VPADStatus status = readVpad();
	switch (axis)
	{
		case 0: return toAxis(status.leftStick.x);
		case 1: return toAxis(-status.leftStick.y);
		case 2: return toAxis(status.rightStick.x);
		case 3: return toAxis(-status.rightStick.y);
		default: return 0;
	}
}

Uint8 SDL_JoystickGetButton(SDL_Joystick*, int button)
{
	const VPADStatus status = readVpad();
	const Uint32 hold = status.hold;

	switch (button)
	{
		case 0: return (hold & VPAD_BUTTON_A) ? 1 : 0;
		case 1: return (hold & VPAD_BUTTON_B) ? 1 : 0;
		case 2: return (hold & VPAD_BUTTON_X) ? 1 : 0;
		case 3: return (hold & VPAD_BUTTON_Y) ? 1 : 0;
		case 4: return (hold & VPAD_BUTTON_MINUS) ? 1 : 0; // back
		case 5: return (hold & VPAD_BUTTON_HOME) ? 1 : 0;  // guide
		case 6: return (hold & VPAD_BUTTON_PLUS) ? 1 : 0;  // start
		case 7: return (hold & VPAD_BUTTON_STICK_L) ? 1 : 0;
		case 8: return (hold & VPAD_BUTTON_STICK_R) ? 1 : 0;
		case 9: return (hold & VPAD_BUTTON_L) ? 1 : 0;
		case 10: return (hold & VPAD_BUTTON_R) ? 1 : 0;
		default: return 0;
	}
}

Uint8 SDL_JoystickGetHat(SDL_Joystick*, int)
{
	const VPADStatus status = readVpad();
	return dpadToHat(status.hold);
}

int SDL_JoystickInstanceID(SDL_Joystick*)
{
	return 0;
}

const char* SDL_JoystickName(SDL_Joystick*)
{
	return "Wii U GamePad";
}

#if SDL_VERSION_ATLEAST(2, 0, 18)
int SDL_JoystickHasRumble(SDL_Joystick*)
{
	return 0;
}

int SDL_JoystickRumble(SDL_Joystick*, Uint16, Uint16, Uint32)
{
	return 0;
}
#endif

int SDL_GameControllerAddMappingsFromFile(const char*)
{
	return 0;
}

SDL_GameController* SDL_GameControllerOpen(int device_index)
{
	static SDL_GameController controller;
	controller.index = device_index;
	return &controller;
}

void SDL_GameControllerClose(SDL_GameController*)
{
}

const char* SDL_GameControllerName(SDL_GameController*)
{
	return "Wii U GamePad";
}

SDL_GameControllerButtonBind SDL_GameControllerGetBindForAxis(SDL_GameController*, SDL_GameControllerAxis axis)
{
	SDL_GameControllerButtonBind bind{};
	bind.bindType = SDL_CONTROLLER_BINDTYPE_AXIS;
	bind.value.axis.axis = static_cast<int>(axis);
	return bind;
}

SDL_GameControllerButtonBind SDL_GameControllerGetBindForButton(SDL_GameController*, SDL_GameControllerButton button)
{
	SDL_GameControllerButtonBind bind{};
	// D-pad uses hat mapping to match SDL expectations.
	switch (button)
	{
		case SDL_CONTROLLER_BUTTON_DPAD_UP:
		case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
		case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
		case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
		{
			bind.bindType = SDL_CONTROLLER_BINDTYPE_HAT;
			bind.value.hat.hat = 0;
			if (button == SDL_CONTROLLER_BUTTON_DPAD_UP) bind.value.hat.hat_mask = SDL_HAT_UP;
			else if (button == SDL_CONTROLLER_BUTTON_DPAD_DOWN) bind.value.hat.hat_mask = SDL_HAT_DOWN;
			else if (button == SDL_CONTROLLER_BUTTON_DPAD_LEFT) bind.value.hat.hat_mask = SDL_HAT_LEFT;
			else bind.value.hat.hat_mask = SDL_HAT_RIGHT;
			return bind;
		}
		default:
		{
			bind.bindType = SDL_CONTROLLER_BINDTYPE_BUTTON;
			bind.value.button.button = static_cast<int>(button);
			return bind;
		}
	}
}

#if SDL_VERSION_ATLEAST(2, 0, 14)
int SDL_GameControllerSetLED(SDL_GameController*, Uint8, Uint8, Uint8)
{
	return 0;
}
#endif

int SDL_GetNumTouchDevices()
{
	return 0;
}

SDL_TouchID SDL_GetTouchDevice(int)
{
	return 0;
}

int SDL_GetNumTouchFingers(SDL_TouchID)
{
	return 0;
}

const SDL_Finger* SDL_GetTouchFinger(SDL_TouchID, int)
{
	return nullptr;
}

SDL_AudioDeviceID SDL_OpenAudioDevice(const char*, int, const SDL_AudioSpec* desired, SDL_AudioSpec* obtained, int)
{
	if (!desired)
		return 0;

	AudioDevice* device = new AudioDevice();
	device->spec = *desired;
	if (obtained)
	{
		*obtained = device->spec;
	}

	const SDL_AudioDeviceID id = gNextAudioId.fetch_add(1);
	gAudioDevices[id] = device;

	device->running = true;
	device->paused = true;
	device->thread = std::thread([device]() {
		const int bytesPerSample = 2;
		const int bufferSize = device->spec.samples * device->spec.channels * bytesPerSample;
		std::vector<Uint8> buffer(bufferSize);

		while (device->running)
		{
			if (device->paused)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
				continue;
			}

			{
				std::lock_guard<std::mutex> lock(device->mutex);
				if (device->spec.callback)
					device->spec.callback(device->spec.userdata, buffer.data(), bufferSize);
			}

			const double seconds = static_cast<double>(device->spec.samples) / static_cast<double>(device->spec.freq);
			const auto sleepMs = static_cast<int>(seconds * 1000.0);
			if (sleepMs > 0)
				std::this_thread::sleep_for(std::chrono::milliseconds(sleepMs));
		}
	});

	return id;
}

void SDL_CloseAudioDevice(SDL_AudioDeviceID dev)
{
	auto it = gAudioDevices.find(dev);
	if (it == gAudioDevices.end())
		return;

	AudioDevice* device = it->second;
	device->running = false;
	if (device->thread.joinable())
		device->thread.join();
	delete device;
	gAudioDevices.erase(it);
}

void SDL_PauseAudioDevice(SDL_AudioDeviceID dev, int pause_on)
{
	auto it = gAudioDevices.find(dev);
	if (it == gAudioDevices.end())
		return;
	it->second->paused = (pause_on != 0);
}

int SDL_GetAudioStatus()
{
	return SDL_AUDIO_PLAYING;
}

int SDL_GetNumAudioDevices(int)
{
	return 1;
}

const char* SDL_GetAudioDeviceName(int, int)
{
	return "Wii U Audio";
}

void SDL_LockAudioDevice(SDL_AudioDeviceID dev)
{
	auto it = gAudioDevices.find(dev);
	if (it == gAudioDevices.end())
		return;
	it->second->mutex.lock();
}

void SDL_UnlockAudioDevice(SDL_AudioDeviceID dev)
{
	auto it = gAudioDevices.find(dev);
	if (it == gAudioDevices.end())
		return;
	it->second->mutex.unlock();
}

namespace
{
	bool readBytes(SDL_RWops* rw, void* out, size_t size)
	{
		return (SDL_RWread(rw, out, 1, size) == size);
	}

	bool readU16LE(SDL_RWops* rw, Uint16& out)
	{
		Uint8 b[2];
		if (!readBytes(rw, b, 2))
			return false;
		out = static_cast<Uint16>(b[0] | (b[1] << 8));
		return true;
	}

	bool readU32LE(SDL_RWops* rw, Uint32& out)
	{
		Uint8 b[4];
		if (!readBytes(rw, b, 4))
			return false;
		out = static_cast<Uint32>(b[0] | (b[1] << 8) | (b[2] << 16) | (b[3] << 24));
		return true;
	}
}

Uint8* SDL_LoadWAV_RW(SDL_RWops* src, int freesrc, SDL_AudioSpec* spec, Uint8** audio_buf, Uint32* audio_len)
{
	if (spec) std::memset(spec, 0, sizeof(SDL_AudioSpec));
	if (audio_buf) *audio_buf = nullptr;
	if (audio_len) *audio_len = 0;
	if (!src)
		return nullptr;

	char riff[4];
	if (!readBytes(src, riff, 4) || std::memcmp(riff, "RIFF", 4) != 0)
	{
		if (freesrc) SDL_RWclose(src);
		return nullptr;
	}

	Uint32 riffSize = 0;
	if (!readU32LE(src, riffSize))
	{
		if (freesrc) SDL_RWclose(src);
		return nullptr;
	}

	char wave[4];
	if (!readBytes(src, wave, 4) || std::memcmp(wave, "WAVE", 4) != 0)
	{
		if (freesrc) SDL_RWclose(src);
		return nullptr;
	}

	Uint16 audioFormat = 0;
	Uint16 numChannels = 0;
	Uint32 sampleRate = 0;
	Uint16 bitsPerSample = 0;
	std::vector<Uint8> dataChunk;

	while (true)
	{
		char chunkId[4];
		if (!readBytes(src, chunkId, 4))
			break;

		Uint32 chunkSize = 0;
		if (!readU32LE(src, chunkSize))
			break;

		if (std::memcmp(chunkId, "fmt ", 4) == 0)
		{
			if (!readU16LE(src, audioFormat) ||
				!readU16LE(src, numChannels) ||
				!readU32LE(src, sampleRate))
				break;

			Uint32 byteRate = 0;
			Uint16 blockAlign = 0;
			if (!readU32LE(src, byteRate) || !readU16LE(src, blockAlign) || !readU16LE(src, bitsPerSample))
				break;

			const Uint32 remaining = (chunkSize > 16) ? (chunkSize - 16) : 0;
			if (remaining > 0)
				SDL_RWseek(src, remaining, RW_SEEK_CUR);
		}
		else if (std::memcmp(chunkId, "data", 4) == 0)
		{
			dataChunk.resize(chunkSize);
			if (chunkSize > 0 && !readBytes(src, dataChunk.data(), chunkSize))
				break;
		}
		else
		{
			// Skip unknown chunk
			SDL_RWseek(src, chunkSize, RW_SEEK_CUR);
		}

		if (!dataChunk.empty() && audioFormat != 0)
			break;
	}

	if (freesrc)
		SDL_RWclose(src);

	if (audioFormat != 1 || dataChunk.empty() || numChannels == 0 || bitsPerSample == 0 || sampleRate == 0)
		return nullptr;

	// Convert to signed 16-bit samples
	const Uint32 bytesPerSample = bitsPerSample / 8;
	const Uint32 numSamples = static_cast<Uint32>(dataChunk.size() / bytesPerSample);
	const Uint32 outBytes = numSamples * 2;
	Uint8* out = static_cast<Uint8*>(std::malloc(outBytes));
	if (!out)
		return nullptr;

	if (bitsPerSample == 8)
	{
		for (Uint32 i = 0; i < numSamples; ++i)
		{
			const int sample = static_cast<int>(dataChunk[i]) - 128;
			const Sint16 s16 = static_cast<Sint16>(sample << 8);
			out[i * 2 + 0] = static_cast<Uint8>(s16 & 0xff);
			out[i * 2 + 1] = static_cast<Uint8>((s16 >> 8) & 0xff);
		}
	}
	else
	{
		for (Uint32 i = 0; i < numSamples; ++i)
		{
			const Uint8 lo = dataChunk[i * 2 + 0];
			const Uint8 hi = dataChunk[i * 2 + 1];
			out[i * 2 + 0] = lo;
			out[i * 2 + 1] = hi;
		}
	}

	if (spec)
	{
		spec->freq = static_cast<int>(sampleRate);
		spec->channels = static_cast<Uint8>(numChannels);
		spec->format = AUDIO_S16LSB;
		spec->samples = 4096;
	}
	if (audio_buf) *audio_buf = out;
	if (audio_len) *audio_len = outBytes;
	return out;
}

Uint8* SDL_LoadWAV(const char* file, SDL_AudioSpec* spec, Uint8** audio_buf, Uint32* audio_len)
{
	SDL_RWops* rw = SDL_RWFromFile(file, "rb");
	return SDL_LoadWAV_RW(rw, 1, spec, audio_buf, audio_len);
}

void SDL_FreeWAV(Uint8* audio_buf)
{
	if (audio_buf)
		std::free(audio_buf);
}

int SDL_BuildAudioCVT(SDL_AudioCVT* cvt, SDL_AudioFormat, Uint8, int, SDL_AudioFormat, Uint8, int)
{
	if (!cvt)
		return -1;
	cvt->needed = 0;
	cvt->len_mult = 1;
	cvt->len_cvt = cvt->len;
	return 0;
}

int SDL_ConvertAudio(SDL_AudioCVT* cvt)
{
	if (!cvt)
		return -1;
	cvt->len_cvt = cvt->len;
	return 0;
}

SDL_RWops* SDL_RWFromFile(const char* file, const char* mode)
{
	SDL_RWops* ops = new SDL_RWops();
	ops->fp = std::fopen(file, mode);
	if (!ops->fp)
	{
		delete ops;
		return nullptr;
	}
	return ops;
}

SDL_RWops* SDL_RWFromFile(const wchar_t* file, const char* mode)
{
#if defined(__GNUC__)
	// WUT uses UTF-8 paths; convert wide -> UTF-8
	std::wstring ws(file ? file : L"");
	std::string utf8(ws.begin(), ws.end());
	return SDL_RWFromFile(utf8.c_str(), mode);
#else
	return SDL_RWFromFile("", mode);
#endif
}

Sint64 SDL_RWsize(SDL_RWops* context)
{
	if (!context || !context->fp)
		return 0;
	const long pos = std::ftell(context->fp);
	std::fseek(context->fp, 0, SEEK_END);
	const long end = std::ftell(context->fp);
	std::fseek(context->fp, pos, SEEK_SET);
	return static_cast<Sint64>(end);
}

Sint64 SDL_RWtell(SDL_RWops* context)
{
	if (!context || !context->fp)
		return 0;
	return static_cast<Sint64>(std::ftell(context->fp));
}

Sint64 SDL_RWseek(SDL_RWops* context, Sint64 offset, int whence)
{
	if (!context || !context->fp)
		return -1;
	return std::fseek(context->fp, static_cast<long>(offset), whence);
}

size_t SDL_RWread(SDL_RWops* context, void* ptr, size_t size, size_t maxnum)
{
	if (!context || !context->fp)
		return 0;
	return std::fread(ptr, size, maxnum, context->fp);
}

size_t SDL_RWwrite(SDL_RWops* context, const void* ptr, size_t size, size_t num)
{
	if (!context || !context->fp)
		return 0;
	return std::fwrite(ptr, size, num, context->fp);
}

int SDL_RWclose(SDL_RWops* context)
{
	if (!context)
		return -1;
	if (context->fp)
		std::fclose(context->fp);
	delete context;
	return 0;
}

int SDL_ShowMessageBox(const SDL_MessageBoxData*, int* buttonid)
{
	if (buttonid) *buttonid = 0;
	return 0;
}

int SDL_ShowSimpleMessageBox(Uint32, const char*, const char*, SDL_Window*)
{
	return 0;
}

int SDL_OpenURL(const char*)
{
	return 0;
}

int SDL_SetClipboardText(const char*)
{
	return 0;
}

SDL_bool SDL_HasClipboardText()
{
	return SDL_FALSE;
}

char* SDL_GetClipboardText()
{
	char* empty = static_cast<char*>(std::malloc(1));
	if (empty) empty[0] = '\0';
	return empty;
}

void SDL_free(void* mem)
{
	std::free(mem);
}

void SDL_Log(const char* message)
{
	if (message)
	{
		std::fprintf(stderr, "%s\n", message);
	}
}

SDL_Thread* SDL_CreateThread(SDL_ThreadFunction fn, const char*, void* data)
{
	SDL_Thread* thread = new SDL_Thread();
	thread->thread = std::thread([fn, data]() { return fn(data); });
	return thread;
}

SDL_Thread* SDL_CreateThreadWithStackSize(SDL_ThreadFunction fn, const char*, size_t, void* data)
{
	return SDL_CreateThread(fn, "", data);
}

void SDL_WaitThread(SDL_Thread* thread, int*)
{
	if (!thread)
		return;
	if (thread->thread.joinable())
		thread->thread.join();
	delete thread;
}

SDL_mutex* SDL_CreateMutex()
{
	return new SDL_mutex();
}

void SDL_DestroyMutex(SDL_mutex* mutex)
{
	delete mutex;
}

void SDL_LockMutex(SDL_mutex* mutex)
{
	if (mutex) mutex->mutex.lock();
}

void SDL_UnlockMutex(SDL_mutex* mutex)
{
	if (mutex) mutex->mutex.unlock();
}

SDL_cond* SDL_CreateCond()
{
	return new SDL_cond();
}

void SDL_DestroyCond(SDL_cond* cond)
{
	delete cond;
}

void SDL_CondSignal(SDL_cond* cond)
{
	if (cond) cond->cond.notify_one();
}

int SDL_CondWaitTimeout(SDL_cond* cond, SDL_mutex* mutex, Uint32 ms)
{
	if (!cond || !mutex)
		return -1;
	std::unique_lock<std::mutex> lock(mutex->mutex, std::adopt_lock);
	cond->cond.wait_for(lock, std::chrono::milliseconds(ms));
	lock.release();
	return 0;
}
