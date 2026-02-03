# Building using Make

**Warning: The Switch build is currently not maintained and may need a manual update of the makefile!**

## Nintendo Switch
1. [Follow the instructions from devkitPro to get your build environment set up.](https://devkitpro.org/wiki/Getting_Started#Setup)
2. Install the following packages using pacman:
```
sudo (dkp-)pacman -Syu
sudo (dkp-)pacman -S switch-pkg-config devkitA64 switch-tools switch-sdl2 switch-glad switch-glm switch-libogg switch-libopus switch-libvorbis switch-libtheora
```
3. In this directory (Oxygen/sonic3air/build/_make):
```
make PLATFORM=Switch
```
Output will be at `bin/Switch/sonic3air.nro`.

## Nintendo Wii U (WUT)
1. Install the Wii U toolchain (recommended meta package):
```
sudo dkp-pacman -Syu
sudo dkp-pacman -S --needed wiiu-dev wiiu-pkg-config
```
2. If you hit missing Ogg/Vorbis/Theora/Zlib/Minizip deps, search for the exact Wii U package names in your repo, then install the ones that exist:
```
dkp-pacman -Ss 'wiiu-.*(ogg|vorbis|theora|zlib|minizip)'
# or list all Wii U portlibs:
dkp-pacman -Sl dkp-libs | rg wiiu
```
3. In this directory (Oxygen/sonic3air/build/_make):
```
make PLATFORM=WiiU
```
Output will be at `bin/WiiU/sonic3air.rpx`.
