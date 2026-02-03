#pragma once
#include <vector>
#include "Geometry.h"

class ShaderUtils {
public:
    static void applySpriteMask(std::vector<Geometry*>& geometries) {
        // OSScreen backend: no shaders needed
    }
};
