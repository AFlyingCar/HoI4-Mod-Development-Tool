#ifndef UTIL_H
#define UTIL_H

#include <cstdint>

#include "Types.h"

namespace MapNormalizer {
    std::uint32_t indexOfLSB(std::uint32_t);
    std::uint32_t swapBytes(std::uint32_t);
    std::uint32_t colorToRGB(const Color&);
    Color RGBToColor(std::uint32_t);

    void ltrim(std::string&);
    void rtrim(std::string&);
    void trim(std::string&);
}

#endif

