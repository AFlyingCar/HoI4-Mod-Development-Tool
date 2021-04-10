/**
 * @file Constants.h
 *
 * @brief Defines various constants used throughout the rest of the program.
 */

#ifndef CONSTANTS_H
# define CONSTANTS_H

# include "Types.h"
# include <cstdint>

namespace MapNormalizer {
    //! Mask to get RED values out of a 24-bit color.
    const std::uint32_t RED_MASK   = 0xFF0000;
    //! Mask to get GREEN values out of a 24-bit color.
    const std::uint32_t GREEN_MASK = 0x00FF00;
    //! Mask to get BLUE values out of a 24-bit color.
    const std::uint32_t BLUE_MASK  = 0x0000FF;
    //! Mask for all color values in a 24-bit color.
    const std::uint32_t COLOR_MASK = 0xFFFFFF;

    //! The magic number marking that a file is a BitMap
    const std::uint16_t BM_TYPE = 19778; // BM

    //! The debug color for finding shapes.
    const Color DEBUG_COLOR = Color{ 255, 255, 255 };

    //! The minimum number of pixels that can be in a valid province
    constexpr size_t MIN_SHAPE_SIZE = 8;
}

#endif

