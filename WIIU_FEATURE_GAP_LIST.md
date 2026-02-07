Wii U Feature Gap List
======================

Purpose
-------

This file lists concrete feature gaps between the PC port and the current Wii U port, mapped to files and suggested work items so contributors can pick tasks to achieve 1:1 parity.

Priority guide
--------------
- P0: Required for renderer to run (shaders, advanced texture formats, GX2 pipeline)
- P1: Required for UI and gameplay parity (now mostly resolved)
- P2: Nice-to-have / performance (GX2 fast-paths, profiling, optimization)

Resolved gaps ✅
-----------------

- ~~librmx/source/wiiu/SDL_shim.cpp (P1)~~
  **Done**: ProcUI lifecycle (WHBProcInit/IsRunning/Shutdown), SDL_PollEvent → SDL_QUIT on HOME, timers, basic RWops, joystick APIs all implemented.

- ~~librmx/source/rmxmedia/audiovideo/AudioManager.cpp (P1)~~
  **Done**: Complete Wii U AudioManager implementation inside `#if defined(PLATFORM_WIIU)` block — dedicated mixing thread, sample conversion, AX voice output. `AudioManager_WiiU.cpp` removed (was duplicate causing linker errors).

- ~~Oxygen/oxygenengine/source/oxygen/platform/PlatformFunctions.cpp (P1)~~
  **Done**: `getAppDataPath()` returns `/vol/external01/S3AIR` on Wii U; creates `scripts/` and `cache/` subdirs at startup.

- ~~Filesystem & RWops mapping (P1)~~
  **Done**: `WiiUFileSystem.cpp` base path `/vol/external01/S3AIR/` with `roms/`, `saves/` subdirs. Unified with PlatformFunctions path.

- ~~Input mapping — VPAD / KPAD (P1)~~
  **Done**: `platform/wiiu/input/` — full VPAD (GamePad) + KPAD (Pro Controller) polling, button mapping to Oxygen engine flags, analog stick → D-pad fallback, touch screen support.

- ~~Networking / Netplay (P2)~~
  **Done**: Netplay disabled at compile time (`#if !defined(PLATFORM_WIIU)` in EngineMain.cpp). `wiiu_network.cpp/.h` provides nsysnet TCP socket wrapper for future use. `wiiu_net::isNetplaySupported()` returns false; clear user-facing message.

- ~~Endianness / byte-order (P1)~~
  **Done**: `RMX_IS_BIG_ENDIAN` macro; `readMemoryUnalignedSwapped` = no-op on BE; BE guards on EmulatorInterface write paths; palette load guard in LemonScriptBindings.

- ~~Embedded ROM loading (P1)~~
  **Done**: `rom_data.h` (4 MB via bin2c.py) → `get_embedded_rom()` hooked into `ResourcesCache::loadRom()`. PlatformSpecifics skips disk ROM check when embedded ROM present.

- ~~ProcUI lifecycle (P1)~~
  **Done**: WHBProcInit/IsRunning/Shutdown integrated into SDL_shim.cpp; HOME button injects SDL_QUIT.

- ~~librmx/source/wiiu/gl_compat.h / gl_compat.cpp — VBO/VAO, buffers, uniforms (P0→P1)~~
  **Done**: Broad implementation covering textures (gen/delete/bind/image2D/subImage2D/texBuffer/readPixels), buffers (gen/delete/bind/data/subData/map/mapRange/unmap/bindRange/bindBase), VAO (gen/bind/delete), vertex attribs (pointer/IPointer/divisor/enable/disable), shader programs (create/delete/attach/detach/compile/link/use/getShaderiv/getProgramiv/getInfoLog), uniforms (1/2/3/4 i/f/iv/fv + matrix 2/3/4), framebuffers, renderbuffers, state (enable/disable/blend/depth/scissor/viewport), draw calls (arrays/elements/instanced/rangeElements), and query functions (glGetString/glGetIntegerv/glGetError).

Remaining gaps
--------------

- Oxygen/oxygenengine/source/oxygen/rendering/opengl/shaders/* (P0)
  - Files: DebugDrawPlaneShader, OpenGLShader, PostFXBlurShader, RenderComponentSpriteShader, RenderPaletteSpriteShader, RenderPlaneShader, RenderVdpSpriteShader, Simple* shaders.
  - Gap: GLSL shaders target desktop GL. Need either a cross-compiler to GX2 shader binary or a GLSL-to-GX2 translation step at build-time.
  - Suggested work: Add a build step to precompile GLSL to GX2 (preferred) or implement minimal CPU-path shaders for critical shaders (fallback). `wiiu_gpu.cpp` ShaderEffect enum already defines CPU-side IDs for each shader type.

- Oxygen/oxygenengine/source/oxygen/rendering/opengl/OpenGLRenderer.cpp (P0)
  - Gap: Uses shader programs with glUseProgram/glGetUniformLocation etc. gl_compat now implements these APIs, but the actual shader execution still falls through to the software rasterizer rather than GX2 hardware.
  - Suggested work: Connect gl_compat shader programs to GX2 shader pipeline; map engine uniform names to GX2 uniform blocks.

- Oxygen/oxygenengine/source/oxygen/rendering/utils/BufferTexture.cpp (P0)
  - Gap: Uses `GL_TEXTURE_BUFFER`. gl_compat implements `glTexBuffer`/`glTexBufferRange` as 1D texture emulation. May need GX2-native buffer texture support for full correctness.
  - Suggested work: Validate current emulation works for all engine use cases; add GX2 buffer texture path if needed.

- librmx/source/rmxmedia/framework/wiiu_shim_gx2.* (P0, P2)
  - Gap: GX2 fast-path covers simple RGBA textures. Additional formats (paletted, compressed, buffer-backed) may be needed for full renderer.
  - Suggested work: Add support for format conversion; expand logging around GX2 init and guard sync/flush points.

- Performance and profiling (P2)
  - Gap: Many CPU fallbacks (software rasterizer in gl_compat, texture uploads, shader emulation). Need real-hardware profiling.
  - Suggested work: Use `wiiu_perf` instrumentation to identify hotspots; offload repeated workloads to GX2. Profile memcpy and blit paths.

Build and test notes
--------------------
- Shader strategy: Prefer precompile GLSL → GX2 binary as a build step. If infeasible, implement small CPU fallbacks for critical shaders (copy, palette, sprite blit) — see `wiiu_gpu.h` ShaderEffect enum.
- Incremental testing: gl_compat now covers VBO/VAO/BufferTexture APIs. Focus on shader pipeline integration next.
- Emulator testing: Use Cemu for iterative development; collect logs from `RMX_LOG_*` to diagnose runtime failures.
- Build output: `bin/WiiU/sonic3air.rpx`

Contributor assignment suggestions
--------------------------------
- Beginner: Implement `SDL_ShowSimpleMessageBox` as OSScreen text overlay; add controller button prompt visuals.
- Intermediate: Connect gl_compat shader programs to GX2 pipeline for a single shader (e.g., copy shader).
- Advanced: Full GX2 shader pipeline for all engine shaders; hardware-accelerated buffer textures.

Next immediate steps (recommended)
--------------------------------
1. Runtime test on Cemu / real hardware and fix any crashes or rendering issues.
2. Implement GX2 shader pipeline for at least the critical copy/palette/sprite shaders (P0).
3. Profile on real hardware and optimize hot paths (P2).

Contact / Questions
-------------------
Open an issue and tag `wiiu` for discussions about large changes (shader pipeline, netplay). Small PRs are preferred and easier to review.
