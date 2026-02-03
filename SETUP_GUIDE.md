# Sonic 3 A.I.R. Wii U - Complete Setup Guide

## Overview

This guide covers setting up the Wii U port of Sonic 3 A.I.R. from building to running on real hardware.

## Building from Source

### Prerequisites
- **DevkitPro** with Wii U support
- **WUT** (Wii U Toolchain)
- **CMake** (optional, for some dependencies)
- **Git** (to clone the repository)

### Environment Setup
```bash
# Set up DevkitPro environment variables
export DEVKITPRO=/opt/devkitpro
export DEVKITPPC=/opt/devkitpro/devkitPPC
export PATH="/opt/devkitpro/devkitPPC/bin:$PATH"

# Verify installation
powerpc-eabi-gcc --version
```

### Build Process
```bash
# Clone the repository
git clone <repository-url>
cd Sonic3AIR-WiiU/sonic3air

# Build the Oxygen engine for Wii U
cd Oxygen/sonic3air/build/_make
make PLATFORM=WiiU -j8

# The build produces:
# - sonic3air.rpx (main executable)
# - Required libraries and resources
```

### Build Troubleshooting
```bash
# Clean build if encountering issues
make clean
make PLATFORM=WiiU -j8

# Common build errors and solutions:
# - Missing headers: Ensure DEVKITPRO is set correctly
# - Link errors: Check WUT installation
# - Permission errors: Verify write permissions in build directory
```

## Installation & File Structure

### Alternative Installation Paths
The port automatically searches this location:
`/vol/external01/S3AIR/`


### ROM File Requirements
- **Filename**: Must be exactly `Sonic_Knuckles_wSonic3.bin`
- **Location**: Place in the same directory as `sonic3air.rpx`
- **Format**: Original Sonic 3 & Knuckles ROM 
- **Size**: Should be approximately 4MB

## Running on Real Hardware

### Aroma Installation
1. **Build the project** using the instructions above
   ```
2. **Launch via Aroma**:
   - Open Aroma on your Wii U
   - Navigate to the S3AIR directory
   - Launch `sonic3air.rpx`

### First Launch
The game should:
1. **Initialize systems** (graphics, audio, input)
2. **Search for ROM file**
3. **Create necessary directories** (save, config, data)
4. **Load game assets**
5. **Enter main menu** (if ROM is found)

## Running on Cemu

### Cemu Setup
1. **Install Cemu** (latest version)
2. **Configure input** (GamePad)
3. **Set graphics settings** (OpenGL or Vulkan)
4. **Mount the S3AIR directory** as a virtual SD card

### Cemu Configuration
```ini
# Recommended Cemu settings for S3AIR
[General]
version = 2

[Graphics]
API = OpenGL  # or Vulkan
UpscaleFilter = Bilinear
UpscaleFactor = 2

[Audio]
AudioLatency = 2

[Account]
PersistentId = 0x80000001
```

## � Controls

### GamePad Controls
- **Left Stick**: Character movement
- **A Button**: Jump
- **B Button**: Spin Dash
- **X/Y Buttons**: Actions (varies by character)
- **L/R Buttons**: Camera control
- **ZL/ZR**: Special abilities
- **Minus**: Pause menu
- **Plus**: Start/Resume

### Keyboard Support (Development)
- **Arrow Keys**: Movement
- **Z**: Jump
- **X**: Spin Dash
- **Enter**: Start
- **ESC**: Pause

## Configuration

### Save Files
- **Location**: `save/` directory in game folder
- **Format**: Binary save data
- **Slots**: Multiple save slots supported
- **Backup**: Automatic backup creation

### Settings File
- **Location**: `config/settings.json`
- **Options**: Graphics, audio, input preferences
- **Reset**: Delete file to restore defaults

### Mod Support
- **Location**: `mods/` directory
- **Format**: Standard Sonic 3 A.I.R. mod format
- **Loading**: Automatic on startup
- **Priority**: Alphabetical order

## Troubleshooting

### Common Issues

#### Build Problems
```bash
# Error: "PLATFORM_WIIU not defined"
export PLATFORM=WiiU

# Error: "Missing WUT headers"
export DEVKITPRO=/opt/devkitpro
export DEVKITPPC=/opt/devkitpro/devkitPPC

# Error: "Permission denied"
chmod +x build/_make/Makefile
```

#### Runtime Issues

**Black Screen on Launch**
- Verify ROM file exists and is correctly named
- Ensure sufficient free space (at least 100MB)

**Input Not Working**
- Check input configuration in `config/settings.json`
- Try different controller if available

**Performance Issues**
- Lower graphics settings in Cemu
- Check SD card speed (real hardware)

### Debug Information
The application creates debug logs:
- **Location**: `logs/` directory
- **Files**: `debug.log`, `error.log`
- **Format**: Plain text with timestamps

### Error Codes
- **0x001**: ROM not found
- **0x002**: Failed to initialize graphics
- **0x003**: Audio system error
- **0x004**: Filesystem access denied

## Performance

### Expected Performance
- **Real Hardware**: 60 FPS target
- **Cemu**: Variable (depends on PC specs)
- **Memory Usage**: ~200MB RAM
- **Storage**: ~500MB for full installation

### Optimization Tips
- Use high-quality SD cards
- Close background applications
- Adjust Cemu settings for your hardware
- Keep ROM file on internal storage if possible

## Updates

### Updating the Port
1. **Backup save files** from `save/` directory
2. **Rebuild from source**
3. **Replace executable** (`sonic3air.rpx`)
4. **Restore save files**
5. **Launch and verify**

### Version Information
- **Check version**: In main menu or `config/version.json`
- **Update notifications**: Check repository releases
- **Compatibility**: Should work with latest Sonic 3 A.I.R. scripts

## Additional Resources

### Documentation
- **Main README**: `README.md` - Project overview
- **Original README**: `READMEog.md` - Upstream documentation
- **Build System**: `Oxygen/sonic3air/build/_make/` - Build files

### Community
- **Discord**: https://discord.gg/y6asn5hQtt (do note that this is also a Discord for other things. it's not just this)
- **GitHub Issues**: Report bugs and request features
- **Forums**: [Community forum link]

### Development
- **Source Code**: Full source available
- **Contributing**: See README.md for guidelines
---

**Note**: This guide is continuously updated as the Wii U port develops. Check for the latest version of this guide in the repository.
