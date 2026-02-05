#!/usr/bin/env bash
set -euo pipefail

echo "Wii U build container initialized."

if command -v dkp-pacman >/dev/null 2>&1; then
    echo "dkp-pacman is installed. You can now install Wii U packages inside the container, for example:"
    echo "  sudo dkp-pacman -S wut ppc-zlib ppc-libogg ppc-libvorbis ppc-libtheora"
    echo "Mount the repository into /workspace and run the Makefile as usual."
else
    echo "dkp-pacman is not installed in this container."
    echo "Follow the DevkitPro instructions inside the container to install dkp-pacman:" 
    echo "  https://devkitpro.org/wiki/Getting_Started"
    echo "After installing devkitpro, run:"
    echo "  sudo dkp-pacman -S wut ppc-zlib ppc-libogg ppc-libvorbis ppc-libtheora"
fi

exec bash
