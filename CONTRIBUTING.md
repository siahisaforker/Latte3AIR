CONTRIBUTING
===========

Thanks for wanting to contribute to Sonic3AIR-WiiU!

This document explains how to get started, common tasks, and where to find contributor-friendly issues.

Getting started
---------------

- Clone the repository and follow the build instructions in Oxygen/sonic3air/build/_make/howtobuild.md.
- The Wii U build requires devkitPPC/wut headers and tools; see SETUP_GUIDE.md for environment setup.
- You MUST provide your own game ROM: `game/Sonic_Knuckles_wSonic3.bin`. The build embeds this file and will abort if missing.

How to contribute
-----------------

- Pick an item from `CONTRIBUTOR_TODO.md` (good-first-issue tasks are marked).
- Create a branch named `wiiu/<short-task-name>`.
- Implement small, well-scoped changes and include a brief test run (e.g., `make PLATFORM=WiiU`).
- Open a pull request describing the change, the rationale, and any manual test steps.

Code style and tests
--------------------

- Follow the existing C++ coding style used in the repo (match indentation, no extra comments).
- Add tests where practical. If your change affects build scripts, update `howtobuild.md`.

Communication
-------------

- Use PR descriptions for discussion. If a change is large, open an issue first to discuss approach.

Files to consult
----------------

- `librmx/source/wiiu/gl_compat.*` — GL compatibility layer for Wii U.
- `librmx/source/wiiu/SDL_shim.cpp` — SDL2 replacement for Wii U.
- `librmx/source/rmxmedia/framework/wiiu_shim_gx2.*` — GX2 texture fast-path and fallbacks.
- `Oxygen/sonic3air/build/_make` — Makefile and build scripts for embedding ROM.
- `CONTRIBUTOR_TODO.md` — short tasks and guidance for contributors.

Thank you — contributions make this port possible!
