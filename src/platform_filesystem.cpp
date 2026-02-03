#include "platform_wiiu.h"
#include <sys/stat.h>

bool ensure_dir_exists(const char* path) {
    struct stat st = {0};
    if (stat(path, &st) == -1) {
        mkdir(path, 0777);
        return true;
    }
    return false;
}

bool load_rom(const char* path, void** buffer, size_t* size) {
    if (!file_exists(path)) return false;
    FILE* f = fopen(path, "rb");
    fseek(f, 0, SEEK_END);
    *size = ftell(f);
    fseek(f, 0, SEEK_SET);
    *buffer = malloc(*size);
    fread(*buffer, 1, *size, f);
    fclose(f);
    return true;
}
