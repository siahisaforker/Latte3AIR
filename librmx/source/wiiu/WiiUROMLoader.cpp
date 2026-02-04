#include "WiiUROMLoader.h"
#include "WiiUFileSystem.h"
#include "WiiUEndianness.h"
#include <fstream>
#include <algorithm>
#include <cstring>

// Accessors provided by `Oxygen/sonic3air/___internal/rom_data.h` via
// `Oxygen/sonic3air/source/sonic3air/___internal/rom_data.cpp` when present.
extern "C" const unsigned char* get_embedded_rom();
extern "C" unsigned int get_embedded_rom_size();

namespace rmx {

bool WiiUROMLoader::mInitialized = false;

bool WiiUROMLoader::initialize()
{
    if (mInitialized)
        return true;

    // Ensure filesystem is initialized
    if (!WiiUFileSystem::initialize())
        return false;

    mInitialized = true;
    return true;
}

void WiiUROMLoader::shutdown()
{
    mInitialized = false;
}

std::vector<WiiUROMLoader::ROMInfo> WiiUROMLoader::findAvailableROMs()
{
    std::vector<ROMInfo> roms;
    
    if (!mInitialized)
        return roms;

    std::string romPath = getDefaultROMPath();
    
    // Specific ROM name for this setup
    std::vector<std::string> romNames = {
        "Sonic_Knuckles_wSonic3.bin"
    };

    for (const auto& romName : romNames)
    {
        std::string fullPath = romPath + romName;
        ROMInfo info = loadROMInfo(fullPath);
        if (info.isValid)
        {
            roms.push_back(info);
        }
    }

    return roms;
}

WiiUROMLoader::ROMInfo WiiUROMLoader::loadROMInfo(const std::string& filePath)
{
    ROMInfo info;
    info.filePath = filePath;
    info.size = 0;
    info.checksum = 0;
    info.isValid = false;

    if (!mInitialized)
        return info;

    std::ifstream file(filePath, std::ios::binary | std::ios::ate);
    if (!file.is_open())
        return info;

    info.size = file.tellg();
    file.seekg(0, std::ios::beg);

    if (!isValidROMSize(info.size))
        return info;

    std::vector<uint8_t> buffer(info.size);
    if (!file.read(reinterpret_cast<char*>(buffer.data()), info.size))
        return info;

    if (!isValidROMFormat(buffer))
        return info;

    info.checksum = calculateChecksum(buffer);
    info.isValid = true;

    return info;
}

std::vector<uint8_t> WiiUROMLoader::loadROMData(const std::string& filePath)
{
    std::vector<uint8_t> romData;

    if (!mInitialized)
        return romData;

    std::ifstream file(filePath, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        // If the ROM file isn't available on disk, try embedded ROM data (if present).
        unsigned int sz = get_embedded_rom_size();
        if (sz > 0) {
            const unsigned char* p = get_embedded_rom();
            if (p) {
                romData.assign(p, p + sz);
                if (validateROM(romData))
                    return romData;
            }
        }
        return romData;
    }

    size_t fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    if (!isValidROMSize(fileSize))
        return romData;

    romData.resize(fileSize);
    if (!file.read(reinterpret_cast<char*>(romData.data()), fileSize))
    {
        romData.clear();
        return romData;
    }

    if (!validateROM(romData))
    {
        romData.clear();
    }

    return romData;
}

bool WiiUROMLoader::validateROM(const std::vector<uint8_t>& romData)
{
    if (romData.empty())
        return false;

    return isValidROMSize(romData.size()) && isValidROMFormat(romData);
}

std::string WiiUROMLoader::getDefaultROMPath()
{
    return WiiUFileSystem::getRomPath();
}

uint32_t WiiUROMLoader::calculateChecksum(const std::vector<uint8_t>& data)
{
    uint32_t checksum = 0;
    for (uint8_t byte : data)
    {
        checksum += byte;
    }
    return checksum;
}

bool WiiUROMLoader::isValidROMSize(size_t size)
{
    // Sonic & Knuckles ROM is typically 2MB (2048 KB) or 4MB (4096 KB)
    // Allow some tolerance for different ROM formats
    return (size >= 1024 * 1024 && size <= 4 * 1024 * 1024);
}

bool WiiUROMLoader::isValidROMFormat(const std::vector<uint8_t>& data)
{
    if (data.size() < 0x100)
        return false;

    // Check for Sega ROM header (ROMs are little-endian, need to swap for big-endian Wii U)
    // "SEGA" at offset 0x100
    if (data.size() >= 0x104)
    {
        // Read as little-endian and convert to big-endian for comparison
        char header[4];
        for (int i = 0; i < 4; i++)
        {
            header[i] = data[0x100 + i];
        }
        
        if (std::memcmp(header, "SEGA", 4) == 0)
        {
            return true;
        }
    }

    // Check for console name "SEGA GENESIS" at offset 0x160
    if (data.size() >= 0x168)
    {
        std::string consoleName(data.begin() + 0x160, data.begin() + 0x168);
        if (consoleName == "SEGA GENESIS")
            return true;
    }

    // Check for "SEGA MEGA DRIVE" at offset 0x160
    if (data.size() >= 0x174)
    {
        std::string consoleName(data.begin() + 0x160, data.begin() + 0x174);
        if (consoleName.find("SEGA MEGA DRIVE") != std::string::npos)
            return true;
    }

    return false;
}

} // namespace rmx
