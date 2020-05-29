
#include "Util.h"

#include "Constants.h"

// Helper replacement for __builtin_ctz if on MSVC
#ifdef MSC_VER
# include <intrin>

// https://stackoverflow.com/a/20468180
static std::uint32_t __inline __builtin_ctz(std::uint32_t value) {
    DWORD trailing_zero = 0;

    if(_BitScanForward(&trailing_zero, value)) return trailing_zero;
    return 0;
}
#endif

// https://stackoverflow.com/a/31393298
std::uint32_t MapNormalizer::indexOfLSB(std::uint32_t value) {
    return __builtin_ctz(value);// + 1;
}

std::uint32_t MapNormalizer::colorToRGB(const Color& color) {
    return ((color.r << 16) | (color.g << 8) | color.b) & COLOR_MASK;
}

MapNormalizer::Color MapNormalizer::RGBToColor(std::uint32_t rgb) {
    return Color{ static_cast<std::uint8_t>((rgb & RED_MASK) >> 16),
                  static_cast<std::uint8_t>((rgb & GREEN_MASK) >> 8),
                  static_cast<std::uint8_t>((rgb & BLUE_MASK))
                };
}

void MapNormalizer::ltrim(std::string& str) {
    str.erase(str.begin(), str.begin() + str.find_first_not_of(" \t\n\r"));
}

void MapNormalizer::rtrim(std::string& str) {
    str.erase(str.begin() + str.find_last_not_of(" \t\n\r") + 1, str.end());
}

void MapNormalizer::trim(std::string& str) {
    ltrim(str);
    rtrim(str);
}

