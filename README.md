# Sonic 3 A.I.R. - Wii U Port

This project aims to be native Wii U port of **Sonic 3 A.I.R.** (Angel Island Revisited), a fan-made remaster of Sonic 3 & Knuckles built on the Oxygen Engine.

## Project Status

**Current State: CPU Shader Pipeline Complete — Runtime Testing Phase**

The Wii U port compiles cleanly to an RPX and all major engine subsystems have been implemented or adapted:

- [x] Core Oxygen Engine builds and links for Wii U (PowerPC / Espresso)
- [x] Big-endian correctness — byte-swaps disabled where M68K data is already native order
- [x] Video output (OSScreen fallback + GX2 via WHBGfx; software renderer active)
- [x] Full GL compatibility layer (`gl_compat`) — textures, buffers, VAO/VBO, shaders, framebuffers, uniforms, draw calls
- [x] Input handling — VPAD (GamePad) + KPAD (Pro Controller) with analog stick → D-pad fallback
- [x] Audio — complete AudioManager mixer running on a dedicated Wii U thread with AX voice output
- [x] Filesystem — SD card paths unified to `/vol/external01/S3AIR/` with `roms/`, `saves/`, `mods/` subdirs
- [x] Embedded ROM loading — 4 MB ROM baked into the RPX via `rom_data.h` (SD card too slow for streaming)
- [x] ProcUI lifecycle — HOME button handling via WHBProcInit/IsRunning/Shutdown in SDL shim
- [x] Networking — netplay disabled at compile time with clear guard; network init skipped on Wii U
- [x] Performance instrumentation — `wiiu_perf` profiling helpers ready for runtime tuning
- [x] Full CPU shader pipeline — all 14 engine GLSL shaders reimplemented in C++ (`wiiu_shaders`)
- [x] GX2 fast-path expansion — buffer textures, palette lookup, multi-texture, FBO render-to-texture
- [ ] Runtime testing and performance optimization on real hardware

## Quick Start

### Prerequisites
- **Wii U with Aroma** (or compatible homebrew environment like Tiramisu)
- **DevkitPro/WUT toolchain** (for building)
- **Original ROM**: `Sonic_Knuckles_wSonic3.bin`

### Building
```bash
# Set up DevkitPro environment
export DEVKITPRO=/opt/devkitpro
export DEVKITPPC=/opt/devkitpro/devkitPPC
export PATH="/opt/devkitpro/devkitPPC/bin:$PATH"

# Build the Oxygen engine for Wii U
cd Oxygen/sonic3air/build/_make
make PLATFORM=WiiU -j8
```

The build produces:
- `sonic3air.rpx` - Main executable
- Required libraries and resources




### Paths
The port automatically tries to detect these locations:

`/vol/external01/S3AIR/roms/` — place your `Sonic_Knuckles_wSonic3.bin` here (only needed if not using the embedded ROM)

`/vol/external01/S3AIR/saves/` — persistent save data is written here automatically

`/vol/external01/S3AIR/mods/` — drop mod folders here

## Features

### Implemented
- **Video**: OSScreen fallback + GX2 (WHBGfx) rendering; broad `gl_compat` layer with software rasterizer
- **Input**: Native Wii U GamePad (VPAD) + Pro Controller (KPAD) support with analog→digital fallback
- **Audio**: Full AudioManager mixer on dedicated thread → AX voice output (sndcore2 backend)
- **Endianness**: Correct big-endian handling — M68K and Wii U share byte order, double-swaps eliminated
- **Filesystem**: Unified SD card paths (`/vol/external01/S3AIR/`) with `roms/`, `saves/`, `mods/` subdirs
- **ROM Loading**: Embedded ROM via `rom_data.h` (4 MB baked into RPX); SD card read as fallback
- **Save System**: Persistent game saves to SD card
- **Mods**: Mod loading from SD card
- **ProcUI**: HOME button lifecycle management (WHBProc integration in SDL shim)
- **Network**: Netplay gracefully disabled with compile-time guard; `wiiu_net` TCP wrapper ready for future use
- **Performance Tools**: `wiiu_perf` instrumentation (scoped timers, per-section reporting)
- **GX2 Renderer**: Basic GX2 rendering backend with textured triangle fast-path
- **OSScreen Renderer**: Fallback framebuffer renderer with text overlay support

### Remaining Work
- **Runtime Testing**: Test on real Wii U hardware or Cemu; validate all 14 CPU shader paths produce correct visuals
- **Performance Optimization**: Profile on real hardware; identify hot shader paths and optimize pixel loops

  
## Technical Details

### Architecture
- **Oxygen Engine**: Core game engine (cross-platform)
- **librmx**: Foundation libraries (media, base utilities)
- **WiiU Layer**: Platform-specific implementations
  - `gl_compat.cpp/.h` - Broad OpenGL compatibility layer (textures, buffers, shaders, FBOs, draw calls)
  - `AudioManager.cpp` - Wii U audio implementation (inside `#if defined(PLATFORM_WIIU)` block)
  - `SDL_shim.cpp` - SDL compatibility layer with ProcUI lifecycle integration
  - `wiiu_shaders.cpp/.h` - CPU shader pipeline (all 14 engine shaders: planes, sprites, post-FX, palette-indexed)
  - `WiiUGfx.cpp` - GX2/OSScreen abstraction with optimized blit
  - `WiiUAudio.h` - AX voice backend declarations
  - `WiiUFileSystem.cpp` - Path mapping to `/vol/external01/S3AIR/`
  - `rom_data.h` / `rom_data.cpp` - Embedded ROM (generated at build time via `bin2c.py`)
- **Platform Layer** (`platform/wiiu/`):
  - `input/` - VPAD/KPAD polling, button mapping, analog stick handling
  - `render/` - GX2 renderer and OSScreen fallback renderer
  - `wiiu_gpu.cpp/.h` - Vertex batch helpers bridging gl_compat → GX2
  - `wiiu_network.cpp/.h` - nsysnet TCP socket wrapper (netplay disabled)
  - `wiiu_perf.cpp/.h` - Performance instrumentation
  - `ax_stubs.cpp` - sndcore2 AX link-time stubs
  - `atomic_stubs.cpp` - libatomic fallbacks

### Build System
- **Platform**: `PLATFORM_WIIU` conditional compilation
- **Toolchain**: WUT (Wii U Toolchain)
- **Makefiles**: Located in `Oxygen/sonic3air/build/_make/`

### Dependencies
- **WUT**: Wii U toolchain
- **WHB**: Wii U homebrew libraries
- **Coreinit**: System APIs (threading, filesystem, etc.)
- **GX2**: Graphics API
- **sndcore2**: Audio API (AX voice output)

## Troubleshooting

### Build Issues

### Embedded ROM handling

 - Place your original ROM `Sonic_Knuckles_wSonic3.bin` in `game/` (repo root: `game/Sonic_Knuckles_wSonic3.bin`).
 - The Wii U Makefile will copy that file into `Oxygen/sonic3air/___internal/` during the build, run `tools/bin2c.py` to generate `Oxygen/sonic3air/___internal/rom_data.h`, and then remove the temporary copy. The original ROM in `game/` is left untouched.
 - The generated header exposes `game_bin_data` and `game_bin_data_size` which the build will include; the runtime uses the embedded bytes in memory.
 - Important: the build will be aborted if the ROM is not present in `game/`. You must provide the original `Sonic_Knuckles_wSonic3.bin` in the `game/` folder. the project will not/cannot run the game without it.
 
 If you prefer to keep the header up-to-date manually, run the `embed-rom` target:

```bash
cd Oxygen/sonic3air/build/_make
make embed-rom
```

This will copy `game/Sonic_Knuckles_wSonic3.bin` into `___internal`, generate `rom_data.h`, and delete the copied binary.
```bash
# If build fails with missing headers:
export DEVKITPRO=/opt/devkitpro
export DEVKITPPC=/opt/devkitpro/devkitPPC

# Clean build:
make clean
make PLATFORM=WiiU -j8
```

### Runtime Issues
- **Black screen**: Check ROM file location and name
- **No input**: Ensure GamePad is connected and initialized
- **Audio issues**: Verify audio files are present

### Common Errors
- `ROM not found`: Ensure `Sonic_Knuckles_wSonic3.bin` is in the correct directory
- `Failed to initialize graphics`: Check if GX2 is avalible (logging is avalible, check the logs to see if GX2 could initialize)
- `Audio device error`: Verify sndcore2 is properly initialized

### Wii U build notes (short)
- Recommended environment: WSL + DevkitPro/WUT installed under `/opt/devkitpro`.
- If you hit missing lib symbols during linking, install the ppc portlibs and add their path:
  - `/opt/devkitpro/portlibs/ppc/lib`
- During recent development the following minimal steps were required to get a working build:
  - Install ppc portlibs (zlib/ogg/vorbis/theora) via `dkp-pacman`.
  - Add `-L/opt/devkitpro/portlibs/ppc/lib` and `-lgcc` to the WiiU platform LIBS in `Oxygen/sonic3air/build/_make/Makefile_cfgs/Platforms/WiiU.cfg`.
  - Provide small platform stubs for missing symbols (`platform/wiiu/atomic_stubs.cpp`, `platform/wiiu/ax_stubs.cpp`) when developing against WUT.
  - Fix Makefile continuation if `make` fails with "recipe commences before first target" (missing backslash on a continued `SOURCES` list entry).
  - Build from WSL: `cd Oxygen/sonic3air/build/_make && make -f Makefile PLATFORM=WiiU -j8`.

Build artifact: `bin/WiiU/sonic3air.rpx` (produced by the build).

## Contributing

### Areas for Contribution
- **Network Layer**: Adapt netplay code for Wii U networking
- **Performance**: Optimize for Wii U hardware constraints
- **Graphics**: Improve GX2 renderer
- **Audio**: Enhance audio quality and features
- **Input**: Add support for additional controllers

### Development Setup
1. Clone this repository
2. Install DevkitPro then get wiiu-dev from pacman
3. Build using the instructions above
4. Test with Cemu or real hardware

## Documentation

- **Original README**: `READMEog.md` - Upstream project documentation
- **Setup Guide**: `SETUP_GUIDE.md` - Detailed setup instructions
- **Build System**: `Oxygen/sonic3air/build/_make/` - Platform-specific build files

## Building for Wii U (Platform-specific)

Follow the platform section that matches your development environment. All platforms require a working DevkitPro/WUT installation and the Wii U portlibs (ppc) installed via `dkp-pacman`.

- Linux

  - Install DevkitPro and WUT following https://devkitpro.org
  - Install Wii U portlibs:

    ```bash
    sudo dkp-pacman -Syu
    sudo dkp-pacman -S wut ppc-zlib ppc-libogg ppc-libvorbis ppc-libtheora
    ```

  - Set environment variables and build from the repo root:

    ```bash
    export DEVKITPRO=/opt/devkitpro
    export DEVKITPPC=$DEVKITPRO/devkitPPC
    export PATH="$DEVKITPPC/bin:$PATH"

    cd Oxygen/sonic3air/build/_make
    make -f Makefile PLATFORM=WiiU -j$(nproc)
    ```

- Windows (WSL recommended)

  - Install WSL 2 and a Linux distribution (Ubuntu recommended).
  - Inside WSL install DevkitPro/WUT as in the Linux section.
  - From Windows PowerShell you can invoke the WSL build command:

    ```powershell
    wsl -e bash -lc "cd '/mnt/c/Users/<you>/Music/Sonic3AIR-WiiU/sonic3air/Oxygen/sonic3air/build/_make' && make -f Makefile PLATFORM=WiiU -j1"
    ```

  - Note: building under native Windows (outside WSL) is not supported due to toolchain and path differences.

- macOS

  - The recommended approach on macOS is to use a Linux VM or a Docker container with DevkitPro/WUT installed, as official macOS packaging may be limited.
  - Alternatively, follow the DevkitPro macOS instructions at https://devkitpro.org and ensure the ppc portlibs are available in your environment.
  - Use the same make invocation inside the prepared environment:

    ```bash
    cd Oxygen/sonic3air/build/_make
    make -f Makefile PLATFORM=WiiU -j8
    ```

If you encounter missing symbols during link, ensure `LIBPATHS` in `Oxygen/sonic3air/build/_make/Makefile_cfgs/Platforms/WiiU.cfg` includes `-L$(DEVKITPRO)/portlibs/ppc/lib` and that required `LIBS` include `-lvorbis -lvorbisfile -logg -ltheora -ltheoradec -lz -latomic -lgcc`.

### Host distributions (common)

This project is commonly built from these host environments; follow the DevkitPro/WUT installer for exact steps, and install the listed utilities via your package manager before installing DevkitPro:

- Ubuntu / Debian (apt):

  ```bash
  sudo apt update
  sudo apt install build-essential git curl python3 cmake pkg-config
  ```

- Fedora / CentOS / RHEL (dnf):

  ```bash
  sudo dnf install @development-tools git curl python3 cmake pkgconfig
  ```

- Arch / Manjaro (pacman):

  ```bash
  sudo pacman -Syu base-devel git curl python cmake pkgconf
  ```

- openSUSE (zypper):

  ```bash
  sudo zypper install -t pattern devel_basis git curl python3 cmake pkg-config
  ```

- macOS (Homebrew / VM):

  ```bash
  brew install git cmake python3 pkg-config
  # Consider using a Linux VM or Docker for best compatibility with WUT
  ```

- Windows (WSL):

  Install WSL2 + a Linux distro (Ubuntu recommended) and follow the Linux steps inside WSL. Building natively on Windows is not supported; use WSL instead.

After installing the basic host packages, follow the DevkitPro/WUT installation guide to install `dkp-pacman`, `wut`, and the Wii U ppc portlibs, then build as described above.

## Automation: Docker & WSL

To simplify creating a reproducible build host we provide a minimal Dockerfile and a WSL helper script in this repository:

- `docker/Dockerfile.wiiu` + `docker/entrypoint-wiiu.sh`: Ubuntu-based container with host build tools installed. The container does not automatically install DevkitPro; run the entrypoint and follow its instructions inside the container to install `dkp-pacman` and Wii U portlibs, then mount the repository and run the normal `make` invocation.

- `scripts/setup_wsl_devkitpro.sh`: Helper script intended for WSL/Ubuntu that installs host prerequisites and, if `dkp-pacman` is already present, installs Wii U portlibs. If `dkp-pacman` is missing the script points you to the DevkitPro install docs.

Example Docker usage:

```bash
docker build -t s3air-wiiu -f docker/Dockerfile.wiiu .
docker run --rm -it -v "$(pwd):/workspace" s3air-wiiu
# inside the container follow the entrypoint instructions to install devkitpro and run make
```

WSL helper (run inside WSL Ubuntu):

```bash
bash scripts/setup_wsl_devkitpro.sh
# After installing devkitpro via the official instructions, run:
# sudo dkp-pacman -S wut ppc-zlib ppc-libogg ppc-libvorbis ppc-libtheora
```


## Legal

This is a non-profit fan project. All Sonic characters and assets belong to SEGA. This project is not affiliated with SEGA or Sonic Team.

**Original Project**: https://sonic3air.org/  
**Wii U Port**: Community-driven adaptation

---

**Note**: The port is code-complete with a full CPU shader pipeline and builds cleanly to `bin/WiiU/sonic3air.rpx`. All 14 engine GLSL shaders have been reimplemented as C++ CPU shaders in `wiiu_shaders.cpp`. Remaining work is runtime testing and performance tuning on real hardware.

**Note 2**: If the build crashes or shows unexpected behavior at runtime, check the logging output — it shows exactly what initializes as it happens. Test on Cemu if needed. Create an Issue if you can't figure it out yourself, or report to the Discord (found in SETUP_GUIDE.md).
