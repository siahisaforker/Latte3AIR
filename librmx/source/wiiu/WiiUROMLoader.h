#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace rmx {

class WiiUROMLoader
{
public:
    struct ROMInfo {
        std::string filePath;
        size_t size;
        uint32_t checksum;
        bool isValid;
    };

    static bool initialize();
    static void shutdown();

    // ROM loading functions
    static std::vector<ROMInfo> findAvailableROMs();
    static ROMInfo loadROMInfo(const std::string& filePath);
    static std::vector<uint8_t> loadROMData(const std::string& filePath);
    static bool validateROM(const std::vector<uint8_t>& romData);

    // Get the default ROM path
    static std::string getDefaultROMPath();

private:
    static uint32_t calculateChecksum(const std::vector<uint8_t>& data);
    static bool isValidROMSize(size_t size);
    static bool isValidROMFormat(const std::vector<uint8_t>& data);

    static bool mInitialized;
};

} // namespace rmx
