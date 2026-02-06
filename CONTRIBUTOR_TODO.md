CONTRIBUTOR_TODO
================

This file lists small, well-scoped tasks that contributors can pick up to help get the Wii U port to feature parity with the PC build.

How to pick a task
------------------

- Start with items labelled `good-first-issue` or `easy`.
- Create a branch `wiiu/<task-name>` and submit a PR when ready.

Good First Tasks
-----------------

- (easy) Implement remaining functions in `librmx/source/wiiu/gl_compat.cpp` used by `BufferTexture.cpp` and shader code. Files:
  - Oxygen/oxygenengine/source/oxygen/rendering/utils/BufferTexture.cpp
  - Oxygen/oxygenengine/source/oxygen/rendering/opengl/shaders/*

- (easy) Implement missing SDL functions in `librmx/source/wiiu/SDL_shim.cpp` used by `PlatformFunctions.cpp` (message boxes, timers, RWops).

- (easy) Add defensive file-open wrappers for Wii U paths and map `mods/`/`saves/` locations to persistent storage.

Intermediate Tasks
------------------

- (medium) Implement VBO/VAO and BufferTexture support in `gl_compat` (glGenBuffers, glBindBuffer, glBufferData, glGenVertexArrays, glBindVertexArray, glVertexAttribPointer, glEnableVertexAttribArray).

- (medium) Add shader support: either cross-compile GLSL to GX2 or implement a shader emulation layer that supports the shaders in `Oxygen/oxygenengine/source/oxygen/rendering/opengl/shaders/`.

- (medium) Verify and extend `librmx/source/rmxmedia/framework/wiiu_shim_gx2.*` for additional texture formats and swizzle logic (RGBA, paletted textures, buffer textures).

Advanced Tasks
--------------

- (hard) Adapt netplay to the Wii U networking stack or provide a robust fallback/disabled mode with clear UI messages.

- (hard) Integrate full audio parity: ensure `AudioManager_WiiU.cpp` via `sndcore2` supports all sample formats, mixing, and latency requirements.

- (hard) Performance: profile on Cemu/Wii U and optimize CPU fallbacks; expand GX2 fast-paths.

Testing & Running
-----------------

- Run the Wii U build with `make PLATFORM=WiiU` from `Oxygen/sonic3air/build/_make`.
- You must provide `game/Sonic_Knuckles_wSonic3.bin`; the build will abort if missing.

Contact
-------

- Open issues and PRs for questions. Tag PRs with `wiiu` and the task category (e.g., `good-first-issue`).
