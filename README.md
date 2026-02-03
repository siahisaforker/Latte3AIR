# Sonic 3 A.I.R. - Wii U Port

A native Wii U port of **Sonic 3 A.I.R.** (Angel Island Revisited), a fan-made remaster of Sonic 3 & Knuckles built on the Oxygen Engine.

## Project Status

**Current State: Active Development**

-Core Oxygen Engine builds for Wii U

-Basic video output (OSScreen fallback + GX2 via WHBGfx)

-Input handling (VPAD/KPAD integration)

-Audio system (sndcore2 backend)

-Filesystem access (SD card support)

-Network (being adapted for Wii U)

-OpenGL renderer (software renderer active)

**Goal: 1:1 feature parity with PC version**

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

`/vol/external01/S3AIR/rom` (rom is where you put your bin)

`/vol/external01/S3AIR/save` (save is where you put your save files)

`/vol/external01/S3AIR/mods` (mods is where you put your mods)

## Features

### Implemented
- **Video**: OSScreen fallback + GX2 (WHBGfx) rendering
- **Input**: Native Wii U GamePad support
- **Audio**: sndcore2-based audio output
- **Filesystem**: Full SD card access
- **Save System**: Persistent game saves
- **Mods**: Basic mod loading support

### In Progress

- **Network**: Adapting netplay for Wii U networking APIs

- **OpenGL**: Full OpenGL renderer support

- **Performance**: Optimizations for Wii U hardware

  
## Technical Details

### Architecture
- **Oxygen Engine**: Core game engine (cross-platform)
- **librmx**: Foundation libraries (media, base utilities)
- **WiiU Layer**: Platform-specific implementations
  - `VideoManager_WiiU.cpp` - Graphics backend
  - `AudioManager_WiiU.cpp` - Audio backend  
  - `SDL_shim.cpp` - SDL compatibility layer
  - `WiiUGfx.cpp` - GX2/OSScreen abstraction

### Build System
- **Platform**: `PLATFORM_WIIU` conditional compilation
- **Toolchain**: WUT (Wii U Toolchain)
- **Makefiles**: Located in `Oxygen/sonic3air/build/_make/`

### Dependencies
- **WUT**: Wii U toolchain
- **WHB**: Wii U homebrew libraries
- **Coreinit**: System APIs (threading, filesystem, etc.)
- **GX2**: Graphics API
- **sndcore2**: Audio API (WIP)

## Troubleshooting

### Build Issues
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

## Legal

This is a non-profit fan project. All Sonic characters and assets belong to SEGA. This project is not affiliated with SEGA or Sonic Team.

**Original Project**: https://sonic3air.org/  
**Wii U Port**: Community-driven adaptation

---

**Note**: This port aims for 1:1 feature parity with the PC version. Some features may be limited by Wii U hardware constraints.



**Note 2**: There are some issues with the Makefile that prevent it from building. I aim to fix that soon, but even if you could build it
it probably won't work. it's in progress.

**Note 3**: if you can get this to compile and run and it doesn't work (or crashes), make note of the logging. It shows exactly what initializes as it is happening, test on cemu if needed. Create an Issue if you can't figure it out yourself.
