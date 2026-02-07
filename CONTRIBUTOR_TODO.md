CONTRIBUTOR_TODO
================

This file lists small, well-scoped tasks that contributors can pick up to help get the Wii U port to feature parity with the PC build.

How to pick a task
------------------

- Start with items labelled `good-first-issue` or `easy`.
- Create a branch `wiiu/<task-name>` and submit a PR when ready.

Completed Tasks ✅
-------------------

- ~~(easy) Implement remaining functions in `gl_compat.cpp` used by `BufferTexture.cpp` and shader code.~~
  **Done**: Broad gl_compat layer now implements textures, buffers (VBO/VAO), shader programs, uniforms, framebuffers, renderbuffers, draw calls (arrays, elements, instanced), state management, and more.

- ~~(easy) Implement missing SDL functions in `SDL_shim.cpp` used by `PlatformFunctions.cpp` (message boxes, timers, RWops).~~
  **Done**: SDL shim covers ProcUI lifecycle (WHBProcInit/IsRunning/Shutdown), event pumping (SDL_PollEvent → SDL_QUIT on HOME), timers, basic RWops, and joystick APIs.

- ~~(easy) Add defensive file-open wrappers for Wii U paths and map `mods/`/`saves/` locations to persistent storage.~~
  **Done**: `WiiUFileSystem.cpp` and `PlatformFunctions.cpp` both map to `/vol/external01/S3AIR/` with `roms/`, `saves/`, `mods/` subdirectories created at startup.

- ~~(medium) Implement VBO/VAO and BufferTexture support in `gl_compat`.~~
  **Done**: glGenBuffers, glBindBuffer, glBufferData, glBufferSubData, glMapBuffer, glMapBufferRange, glUnmapBuffer, glGenVertexArrays, glBindVertexArray, glDeleteVertexArrays, glVertexAttribPointer, glVertexAttribIPointer, glVertexAttribDivisor, glEnableVertexAttribArray all implemented.

- ~~(hard) Adapt netplay to the Wii U networking stack or provide a robust fallback/disabled mode with clear UI messages.~~
  **Done**: Netplay disabled at compile time in `EngineMain.cpp` (`#if !defined(PLATFORM_WIIU)`). `wiiu_network.cpp/.h` provides TCP socket wrapper via nsysnet for future use. Clear status message returned by `wiiu_net::getNetplayStatusMessage()`.

- ~~(hard) Integrate full audio parity via sndcore2.~~
  **Done**: Complete AudioManager implementation in `AudioManager.cpp` (inside `#if defined(PLATFORM_WIIU)` block) with dedicated mixing thread, sample format conversion, AX voice output, and `::std::swap` namespace fix. `AudioManager_WiiU.cpp` removed (was duplicate). `ax_stubs.cpp` provides full sndcore2 link-time stubs with proper state tracking.

- ~~(hard) Endianness / byte-order correctness.~~
  **Done**: `RMX_IS_BIG_ENDIAN` macro in `Types.h`; `readMemoryUnalignedSwapped` = no-op on big-endian; BE guards on EmulatorInterface write paths; palette load guard in LemonScriptBindings.

- ~~(hard) Embedded ROM loading.~~
  **Done**: `rom_data.h` (4 MB C array via bin2c.py) → `get_embedded_rom()` / `get_embedded_rom_size()` → hooked into `ResourcesCache::loadRom()` before any filesystem paths. `PlatformSpecifics` skips ROM-on-disk check when embedded ROM exists.

- ~~(medium) ProcUI lifecycle integration.~~
  **Done**: `SDL_shim.cpp` calls WHBProcInit() on SDL_Init, WHBProcShutdown() on SDL_Quit; SDL_PollEvent checks WHBProcIsRunning() and injects SDL_QUIT on HOME press.

Good First Tasks (remaining)
-----------------------------

- (easy) Implement `SDL_ShowSimpleMessageBox` as an in-game dialog or OSScreen text overlay for error reporting on Wii U.

- (easy) Add controller visual style selection (GamePad vs Pro Controller button prompts) in `DynamicSprites`.

Intermediate Tasks (remaining)
-------------------------------

- (medium) Add shader support: cross-compile GLSL to GX2 shader binaries at build time, or implement CPU-path shader emulation for critical shaders (copy, palette, sprite blit).

- (medium) Extend `wiiu_shim_gx2.*` for additional texture formats (paletted textures, compressed formats, buffer textures) and swizzle/tile logic.

Advanced Tasks (remaining)
---------------------------

- (hard) Performance: profile on real Wii U hardware and Cemu; optimize CPU fallbacks in `gl_compat`; expand GX2 fast-paths to reduce software rasterizer usage.

- (hard) Implement full GX2 shader pipeline for all engine shaders (`RenderPlaneShader`, `RenderVdpSpriteShader`, `PostFXBlurShader`, etc.).

Testing & Running
-----------------

- Run the Wii U build with `make PLATFORM=WiiU` from `Oxygen/sonic3air/build/_make`.
- Build output: `bin/WiiU/sonic3air.rpx`.
- You must provide `game/Sonic_Knuckles_wSonic3.bin`; the build will abort if missing.

Contact
-------

- Open issues and PRs for questions. Tag PRs with `wiiu` and the task category (e.g., `good-first-issue`).
