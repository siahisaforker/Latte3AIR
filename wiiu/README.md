Wii U build & packaging notes
=============================

Build (devkitPPC + WUT):

1. Install devkitPPC and WUT development files for your environment.
2. From the repository root run:

```bash
make -C wiiu
```

This will attempt to build `s3air_wiiu.elf` using the `wiiu/Makefile` scaffold. Adjust include paths and linker script as needed.


UPDATE: forgot aroma can't run elf homebrew, fix coming soon


Packaging:

Run the helper script to stage the ELF and assets and optionally call `whb_pack`:

```bash
./wiiu/package_wuhb.sh
```

If `whb_pack` is not available, the script will create an uncompressed package directory at `wiiu/wiiu_build/wuhb` — use your preferred tool to create a WUHB from that directory.

Assets:

Place the `S3AIR` folder with `Sonic_Knuckles_wSonic3.bin` on the root of the SD card (common mount points: `sd:/S3AIR`, `/vol/storage_sd/S3AIR`, `/vol/storage_mlc01/S3AIR`).

Runtime behavior:

- The game looks for `Sonic_Knuckles_wSonic3.bin` in the working directory and will abort startup with an error message if it is missing.
- Audio, input, and rendering currently use platform-specific stubs; please follow TODOs in source files to integrate WUT/WHB APIs.
