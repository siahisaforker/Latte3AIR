Wii U Feature Gap List
======================

Purpose
-------

This file lists concrete feature gaps between the PC port and the current Wii U port, mapped to files and suggested work items so contributors can pick tasks to achieve 1:1 parity.

Priority guide
--------------
- P0: Required for renderer to run (shaders, VBO/VAO, uniforms, buffer textures)
- P1: Required for UI and gameplay parity (SDL dialog/timers, audio mixing, input mapping)
- P2: Nice-to-have / performance (netplay adaptation, advanced GX2 fast-paths, profiling)

Per-file gaps and recommended work
---------------------------------

- Oxygen/oxygenengine/source/oxygen/rendering/opengl/shaders/*  (P0)
  - Files: DebugDrawPlaneShader, OpenGLShader, PostFXBlurShader, RenderComponentSpriteShader, RenderPaletteSpriteShader, RenderPlaneShader, RenderVdpSpriteShader, Simple* shaders.
  - Gap: GLSL shaders target desktop GL. Need either a cross-compiler to GX2 shader binary or a GLSL-to-GX2 translation step at build-time.
  - Suggested work: Add a build step to precompile GLSL to GX2 (preferred) or implement minimal CPU-path shaders for critical shaders (fallback).

- Oxygen/oxygenengine/source/oxygen/rendering/opengl/OpenGLRenderer.cpp (P0)
  - Gap: Uses shader programs, program/attribute/uniform binds, glUseProgram, glGetUniformLocation, etc.
  - Suggested work: Ensure `gl_compat` implements program, shader, uniform APIs or route to GX2 pipeline calls; verify uniform types used by each shader.

- Oxygen/oxygenengine/source/oxygen/rendering/utils/BufferTexture.cpp (P0)
  - Gap: Uses `GL_TEXTURE_BUFFER`, buffer textures, glGenBuffers, glBindBuffer, glBufferData. `gl_compat` must implement buffer object backing and map to GX2 or emulate with texture2D.
  - Suggested work: Implement `glGenBuffers`/`glBindBuffer`/`glBufferData` in `librmx/source/wiiu/gl_compat.*`. Provide a BufferTexture fallback that uploads to a 2D texture if buffer textures unsupported.

- librmx/source/wiiu/gl_compat.h / gl_compat.cpp (P0,P1)
  - Gap: Header lists many GL functions; many need full implementations: shader compile/link, program objects, attribute/uniform handling, VBO/VAO, buffer ranges, texture formats, glReadPixels, glDrawElements, instancing.
  - Suggested work: Prioritize VBO/VAO, glBufferData, glDrawElements, shader program management and uniform setters. Implement texture format translation for common formats (GL_RGBA, GL_RGB, GL_UNSIGNED_BYTE, paletted formats used by engine).

- librmx/source/rmxmedia/framework/wiiu_shim_gx2.* (P0,P2)
  - Gap: GX2 fast-path implemented for simple textures, but additional formats (buffer textures, paletted textures, compressed formats) may be needed. Swizzle/tile handling must match texture uploads.
  - Suggested work: Add support for buffer-backed textures and format conversion; expand logging around GX2 init and guard sync/flush points.

- librmx/source/wiiu/SDL_shim.cpp (P1)
  - Gap: `PlatformFunctions.cpp` uses SDL message boxes, timers, RWops, audio open semantics; ensure shim implements `SDL_ShowSimpleMessageBox`, `SDL_Delay`/`SDL_GetTicks`, `SDL_RWFromFile`, `SDL_OpenAudioDevice`, `SDL_LoadWAV_RW`, and controller APIs used by engine.
  - Suggested work: Implement missing SDL calls used by `PlatformFunctions.cpp`; add robust error returns and log messages. Add path mapping for persistent saves/mods.

- Oxygen/oxygenengine/source/oxygen/platform/PlatformFunctions.cpp (P1)
  - Gap: Uses SDL message boxes and file dialogs; verify these APIs are functional on Wii U or provide no-op/console fallbacks.
  - Suggested work: Ensure UI fallbacks for Wii U (simple in-game dialogs) if message boxes not available.

- librmx/source/rmxmedia/audiovideo/AudioManager_WiiU.cpp and librmx/source/wiiu/WiiUAudio.h (P1)
  - Gap: `sndcore2` backend must support mixing, sample formats, device selection, and callback timing similar to SDL audio expectations.
  - Suggested work: Validate `AudioManager_WiiU` handles same sample rates, stereo/mono conversions, and low-latency callback behavior; add stress tests for audio buffer underrun/overrun.

- Input mapping (VPAD / keyboard abstraction) (P1)
  - Files: `Oxygen/sonic3air/source/sonic3air/*` (input code, DynamicSprites for gamepad visuals)
  - Gap: Ensure `VPAD` mapping covers keyboard bindings, multiple controllers, and controller visual styles.
  - Suggested work: Create a mapping layer that maps engine input events to VPAD button sets and add settings for GamePad style.

- Filesystem & RWops mapping (P1)
  - Gap: `mods/`, `saves/`, RWops-based resource loads assume block FS. Wii U must map those to persistent areas (Saves, SD paths) and ensure return paths for writes.
  - Suggested work: Implement `PlatformFile` wrappers that translate paths and add unit tests accessing mods/saves.

- Networking / Netplay (P2)
  - Files: `Oxygen/sonic3air/source/sonic3air/EngineDelegate.*` and netplay stack
  - Gap: Wii U networking APIs differ; netplay may need rework or be disabled with UI notice.
  - Suggested work: Stub netplay endpoints with clear UI message and plan a later port to Wii U sockets APIs.

- Performance and profiling (P2)
  - Gap: Many CPU fallbacks (software texture uploads, swizzling, shader emulation) are slower. Identify memcpy hotspots and optimize.
  - Suggested work: Add instrumentation hooks in `gl_compat` and `wiiu_shim_gx2` to measure upload times; prioritize offload of repeated workloads to GX2.

Build and test notes
--------------------
- Shader strategy: Prefer precompile GLSL -> GX2 binary as a build step. If that's infeasible, implement a small CPU fallback for critical shaders (copy, palette, sprite blit).
- Incremental testing: Start by implementing `glGenBuffers`/`glBindBuffer`/`glBufferData` and buffer-to-2D fallback to get `BufferTexture.cpp` building and rendering simple quads. Then add shader program stubs.
- Emulator testing: Use Cemu for iterative development; collect logs from `RMX_LOG_*` to diagnose runtime failures.

Contributor assignment suggestions
--------------------------------
- Beginner: Implement SDL shim calls used by `PlatformFunctions.cpp` and path mapping for saves/mods.
- Intermediate: Implement VBO/BufferTexture mapping in `gl_compat` and test `BufferTexture.cpp` usage.
- Advanced: Add shader cross-compilation step or implement full GX2 shader pipeline for all engine shaders.

Next immediate steps (recommended)
--------------------------------
1. Implement buffer object support and BufferTexture fallback in `librmx/source/wiiu/gl_compat.*` (P0).
2. Add a minimal shader program stub and uniform setters so renderer can render without full GX2 shaders (temporary P0 fallback).
3. Implement missing SDL shim functions used by `PlatformFunctions.cpp` (P1).

Contact / Questions
-------------------
Open an issue and tag `wiiu` for discussions about large changes (shader pipeline, netplay). Small PRs are preferred and easier to review.
