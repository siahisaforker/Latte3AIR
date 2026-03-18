# Sonic 3 A.I.R. - Wii U Port

This project aims to be native Wii U port of **Sonic 3 A.I.R.** (Angel Island Revisited), a fan-made remaster of Sonic 3 & Knuckles built on the Oxygen Engine.



## Quick Start 1 (releases tab) - NOT AVALIBLE YET
Grab an RPX or wuhb from the releases page.

Place RPX or wuhb in apps folder

Extract Sonic3AIR to the root of your SD Card, place "Sonic_Knuckles_wSonic3.bin" in the "data" folder


**HUGE THANKS TO THE AROMA DISCORD**
## Project Status

In the final stages, just need to correct redness and debug some soft crashes (exit back to the main menu, NO clue why this happens)

## Quick Start 2 (manual building, reccomended)

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
make PLATFORM=WiiU (-j8 as an example, the number of cores you want to use is what number to put after j)
```

The build produces:
- `sonic3air.rpx` - Executable


### Paths
The port automatically tries to detect these locations:

`/vol/external01/Sonic3AIR/data/` — place your `Sonic_Knuckles_wSonic3.bin` here (if not using embedded ROM), build the windows version and put audiodata.bin/enginedata.bin/gamedata.bin/metadata.json (rename to content.json)/scripts.bin

`/vol/external01/Sonic3AIR/saves/` — persistent save data is written here automatically

-comment, this is the legacy save location and is no longer relevant. Save data goes into the same folder as the rom

`/vol/external01/Sonic3AIR/mods/` — drop mod folders here (mods that alter shader behavior may not work, if those even exist. use with caution)

  
## Technical Details

### Architecture
- **Oxygen Engine**: Core game engine (cross-platform)
- **librmx**: Foundation libraries (media, base utilities)
- **WiiU Layer**: Platform-specific implementations
  - `gl_compat.cpp/.h` - Broad OpenGL compatibility layer (textures, buffers, shaders, FBOs, draw calls)
  - `AudioManager.cpp` - Wii U audio implementation (inside `#if defined(PLATFORM_WIIU)` block)
  - `SDL_shim.cpp` - SDL compatibility layer with ProcUI lifecycle integration
  - `wiiu_shaders.cpp/.h` - CPU shader pipeline (all 14 engine shaders: planes, sprites, post-FX, palette-indexed)
  - `WiiUGfx.cpp` - GX2/OSScreen abstraction with optimized blit (disabled when SDL2-WiiU is avalible, used as fast path)
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
- **Black screen**: Check if ROM is in the correct path
- **No input**: Ensure your controller is connected before launching game

### Common Errors
- `ROM not found`: Ensure `Sonic_Knuckles_wSonic3.bin` is in the correct directory


### Areas for Contribution

-̶ ̶*̶*̶N̶e̶t̶w̶o̶r̶k̶ ̶L̶a̶y̶e̶r̶*̶*̶:̶ ̶A̶d̶a̶p̶t̶ ̶n̶e̶t̶p̶l̶a̶y̶ ̶c̶o̶d̶e̶ ̶f̶o̶r̶ ̶W̶i̶i̶ ̶U̶ ̶n̶e̶t̶w̶o̶r̶k̶i̶n̶g̶

̶-̶ ̶*̶*̶P̶e̶r̶f̶o̶r̶m̶a̶n̶c̶e̶*̶*̶:̶ ̶O̶p̶t̶i̶m̶i̶z̶e̶ ̶f̶o̶r̶ ̶W̶i̶i̶ ̶U̶ ̶h̶a̶r̶d̶w̶a̶r̶e̶ ̶c̶o̶n̶s̶t̶r̶a̶i̶n̶t̶s̶

̶-̶ ̶*̶*̶G̶r̶a̶p̶h̶i̶c̶s̶*̶*̶:̶ ̶I̶m̶p̶r̶o̶v̶e̶ ̶G̶X̶2̶ ̶r̶e̶n̶d̶e̶r̶e̶r̶

̶-̶ ̶*̶*̶A̶u̶d̶i̶o̶*̶*̶:̶ ̶E̶n̶h̶a̶n̶c̶e̶ ̶a̶u̶d̶i̶o̶ ̶q̶u̶a̶l̶i̶t̶y̶ ̶a̶n̶d̶ ̶f̶e̶a̶t̶u̶r̶e̶s̶

-̶ ̶*̶*̶I̶n̶p̶u̶t̶*̶*̶:̶ ̶A̶d̶d̶ ̶s̶u̶p̶p̶o̶r̶t̶ ̶f̶o̶r̶ ̶a̶d̶d̶i̶t̶i̶o̶n̶a̶l̶ ̶c̶o̶n̶t̶r̶o̶l̶l̶e̶r̶s̶

-- **Optimization:** Optimize GX2 fast paths and identify redness cause

### Development Setup
1. Clone this repository
2. Install DevkitPro then get wiiu-dev from pacman
3. Build using the instructions above

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

---

**Note**: The game currently gets past the disclaimer screen, but actual gameplay doesn't work as of now. I'm working on a fix for this

**Note 2**: If the build crashes or shows unexpected behavior at runtime, check the logging output, it shows exactly what initializes as it happens. Test on Cemu if needed. Create an Issue if you can't figure it out yourself, or report to the Discord (found in SETUP_GUIDE.md).

**Note 3***: ROM Data is loaded into RAM because 1. it's much faster then loading from SD (plus, some sd cards might not be fast enough) 2. Easier for the user and me for testing. Load from SD card is a fallback and is not ideal.
