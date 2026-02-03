#!/usr/bin/env bash
# Simple packaging helper for Wii U homebrew (WUHB) packaging.
# Adjust paths for your devkit/WUT installation.

set -e

ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
BUILD_DIR="$ROOT_DIR"
OUTPUT_DIR="$ROOT_DIR/wiiu_build"
ELF_NAME="s3air_wiiu.elf"
PKG_DIR="$OUTPUT_DIR/wuhb"

mkdir -p "$PKG_DIR"

if [ ! -f "$BUILD_DIR/$ELF_NAME" ]; then
  echo "Error: ELF not found at $BUILD_DIR/$ELF_NAME"
  echo "Build the ELF first with the wiiu Makefile." >&2
  exit 1
fi

cp "$BUILD_DIR/$ELF_NAME" "$PKG_DIR/"

# Copy assets (S3AIR) if present
if [ -d "$ROOT_DIR/S3AIR" ]; then
  echo "Copying S3AIR assets..."
  cp -r "$ROOT_DIR/S3AIR" "$PKG_DIR/"
fi

echo "Packaging into WUHB directory: $PKG_DIR"
echo "If you have WUT's whb_pack tool, run it now to create a loadable WUHB." 
echo "Example: whb_pack -o S3AIR.wuhb -i $PKG_DIR" 

if command -v whb_pack >/dev/null 2>&1; then
  whb_pack -o "$OUTPUT_DIR/S3AIR.wuhb" -i "$PKG_DIR"
  echo "Created $OUTPUT_DIR/S3AIR.wuhb"
else
  echo "whb_pack not found: created uncompressed package at $PKG_DIR" 
fi
