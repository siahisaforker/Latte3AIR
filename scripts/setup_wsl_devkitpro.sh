#!/usr/bin/env bash
set -euo pipefail

echo "Preparing WSL environment for Wii U builds (Ubuntu/Debian)"

if [ "$(id -u)" -ne 0 ]; then
    SUDO=sudo
else
    SUDO=
fi

echo "Installing host prerequisites (build-essential, git, curl, cmake, pkg-config)..."
${SUDO} apt-get update
${SUDO} apt-get install -y build-essential git curl ca-certificates python3 cmake pkg-config wget unzip xz-utils

if command -v dkp-pacman >/dev/null 2>&1; then
    echo "dkp-pacman already installed. Installing Wii U portlibs..."
    ${SUDO} dkp-pacman -S --noconfirm wut ppc-zlib ppc-libogg ppc-libvorbis ppc-libtheora
    echo "Finished installing Wii U packages via dkp-pacman."
else
    echo "dkp-pacman (devkitPro pacman) is not found on this system."
    echo "Please follow the official DevkitPro installation instructions:" 
    echo "  https://devkitpro.org/wiki/Getting_Started"
    echo "After installing dkp-pacman, run (inside WSL):"
    echo "  sudo dkp-pacman -S wut ppc-zlib ppc-libogg ppc-libvorbis ppc-libtheora"
fi

echo "WSL setup script finished. Ensure DEVKITPRO and DEVKITPPC env vars are set, then build as described in SETUP_GUIDE.md." 
