/**
 * @file Util.h
 *
 * @brief Defines various utility functions.
 */

#ifndef UTIL_H
# define UTIL_H

# include <cstdint>
# include <algorithm>

# include "Types.h"

namespace MapNormalizer {
    // Forward declare this, as we don't need to include the whole file yet.
    struct BitMap;

    std::uint32_t indexOfLSB(std::uint32_t);
    std::uint32_t swapBytes(std::uint32_t);
    std::uint32_t colorToRGB(const Color&);
    Color RGBToColor(std::uint32_t);
    bool doColorsMatch(const Color&, const Color&);

    uint64_t xyToIndex(BitMap*, uint32_t, uint32_t);
    uint64_t xyToIndex(uint32_t, uint32_t, uint32_t);
    Color getColorAt(BitMap*, uint32_t, uint32_t, uint32_t = 3);
    Pixel getAsPixel(BitMap*, uint32_t, uint32_t, uint32_t = 3);

    bool isInImage(BitMap*, uint32_t, uint32_t);

    bool isShapeTooLarge(uint32_t, uint32_t, BitMap*);
    std::pair<uint32_t, uint32_t> calcShapeDims(const Polygon&);

    void ltrim(std::string&);
    void rtrim(std::string&);
    void trim(std::string&);

    template<typename T>
    T clamp(T val, T min, T max) {
        return std::min(std::max(val, min), max);
    }

    void writeColorTo(unsigned char*, uint32_t, uint32_t, uint32_t, Color);
}

#endif
