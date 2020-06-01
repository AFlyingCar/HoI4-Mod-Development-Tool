#ifndef CONSTANTS_H
#define CONSTANTS_H

#include "Types.h"
#include <cstdint>

namespace MapNormalizer {
    const std::uint32_t RED_MASK   = 0xFF0000;
    const std::uint32_t GREEN_MASK = 0x00FF00;
    const std::uint32_t BLUE_MASK  = 0x0000FF;
    const std::uint32_t COLOR_MASK = 0xFFFFFF;

    const std::uint16_t BM_TYPE = 19778; // BM

    const Color DEBUG_COLOR = Color{ 255, 255, 255 };
}

#endif

