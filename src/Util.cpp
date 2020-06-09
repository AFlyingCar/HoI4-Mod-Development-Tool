
#include "Util.h"

#include "Constants.h"
#include "BitMap.h"

// Helper replacement for __builtin_ctz if on MSVC
#ifdef MSC_VER
# include <intrin>

// https://stackoverflow.com/a/20468180
static std::uint32_t __inline __builtin_ctz(std::uint32_t value) {
    DWORD trailing_zero = 0;

    if(_BitScanForward(&trailing_zero, value)) return trailing_zero;
    return 0;
}

// https://stackoverflow.com/a/105339
static std::uint32_t __inline __builtin_bswap32(std::uint32_t value) {
    return _byteswap_ulong(value);
}

#endif

// https://stackoverflow.com/a/31393298
std::uint32_t MapNormalizer::indexOfLSB(std::uint32_t value) {
    return __builtin_ctz(value);// + 1;
}

std::uint32_t MapNormalizer::swapBytes(std::uint32_t value) {
    return __builtin_bswap32(value);
}

std::uint32_t MapNormalizer::colorToRGB(const Color& color) {
    return ((color.g << 8) | (color.r << 16) | color.b) & COLOR_MASK;
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

/**
 * @brief Converts the given XY coordinate to a flat index value
 *
 * @param image The image to index into
 * @param x The X coordinate to convert
 * @param y The Y coordinate to convert
 *
 * @return The given XY coordinate as a flat index
 */
uint64_t MapNormalizer::xyToIndex(BitMap* image, uint32_t x, uint32_t y) {
    return x + (y * image->info_header.width);
}

/**
 * @brief Converts the given XY coordinate to a flat index value
 *
 * @param w The width of the image
 * @param x The X coordinate to convert
 * @param y The Y coordinate to convert
 *
 * @return The given XY coordinate as a flat index
 */
uint64_t MapNormalizer::xyToIndex(uint32_t w, uint32_t x, uint32_t y) {
    return x + (y * w);
}

/**
 * @brief Gets the given XY coordinate as a Pixel from the given BitMap
 *
 * @param image The BitMap to get a pixel from
 * @param x The X coordinate to use
 * @param y The Y coordinate to use
 *
 * @return All data related to the given XY coordinate as a Pixel
 */
MapNormalizer::Pixel MapNormalizer::getAsPixel(BitMap* image, uint32_t x,
                                               uint32_t y, uint32_t depth)
{
    auto index = xyToIndex(image->info_header.width * depth, x * depth, y);

    return Pixel {
        { x, y },
        { image->data[index],
          image->data[index + 1],
          image->data[index + 2]
        }
    };
}

