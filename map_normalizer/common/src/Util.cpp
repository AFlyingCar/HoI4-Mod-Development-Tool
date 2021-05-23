
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
uint64_t MapNormalizer::xyToIndex(const BitMap* image, uint32_t x, uint32_t y) {
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
 * @brief Gets the color at the given XY coordinate from the given BitMap
 *
 * @paramm image The BitMap to get a color from
 * @param x The X coordinate to use
 * @param y The Y coordinate to use
 * @param depth The BitMap's color depth
 *
 * @return An RGB color from image at (x,y)
 */
MapNormalizer::Color MapNormalizer::getColorAt(const BitMap* image, uint32_t x,
                                               uint32_t y, uint32_t depth)
{
    auto index = xyToIndex(image->info_header.width * depth, x * depth, y);

    return { image->data[index],
             image->data[index + 1],
             image->data[index + 2]
           };
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
MapNormalizer::Pixel MapNormalizer::getAsPixel(const BitMap* image, uint32_t x,
                                               uint32_t y, uint32_t depth)
{
    return Pixel {
        { x, y },
        getColorAt(image, x, y, depth)
    };
}

/**
 * @brief Checks if two colors match
 *
 * @param c1 The first color
 * @param c2 The second color
 *
 * @return True if c1 matches c2
 */
bool MapNormalizer::doColorsMatch(const Color& c1, const Color& c2) {
    return c1.r == c2.r && c1.g == c2.g && c1.b == c2.b;
}

/**
 * @brief Checks if a given point is within the bounds of the image.
 *
 * @param image The image
 * @param x The x coordinate to check
 * @param y The y coordinate to check
 *
 * @return True if the point is within the bounds of the image, false otherwise
 */
bool MapNormalizer::isInImage(const BitMap* image, uint32_t x, uint32_t y) {
    return x >= 0 && x < static_cast<uint32_t>(image->info_header.width) &&
           y >= 0 && y < static_cast<uint32_t>(image->info_header.height);
}

bool MapNormalizer::isShapeTooLarge(uint32_t s_width, uint32_t s_height,
                                    const BitMap* image)
{
    auto i_width = image->info_header.width;
    auto i_height = image->info_header.height;

    // shape width/height >= (1/8) of the total map width/height
    return static_cast<float>(s_width) >= (i_width / 8.0f) ||
           static_cast<float>(s_height) >= (i_height / 8.0f);
}

/**
 * @brief Calculates the width and height of the given boundingg box
 *
 * @param bb The bounding box
 *
 * @return A pair containing the width and height of the bounding box
 */
std::pair<uint32_t, uint32_t> MapNormalizer::calcDims(const BoundingBox& bb)
{
    return std::make_pair(static_cast<uint32_t>(std::abs(static_cast<int>(bb.top_right.x) - static_cast<int>(bb.bottom_left.x))),
                          static_cast<uint32_t>(std::abs(static_cast<int>(bb.top_right.y) - static_cast<int>(bb.bottom_left.y))));
}

/**
 * @brief Calculates the width and height of the given shape
 *
 * @param shape The shape
 *
 * @return A pair containing the width and height of the shape
 */
std::pair<uint32_t, uint32_t> MapNormalizer::calcShapeDims(const Polygon& shape)
{
    return calcDims(shape.bounding_box);
}

void MapNormalizer::writeColorTo(unsigned char* color_data, uint32_t w,
                                 uint32_t x, uint32_t y, Color c)
{
    uint32_t index = xyToIndex(w * 3, x * 3, y);

    // Make sure we swap B and R (because BMP format sucks)
    color_data[index] = c.b;
    color_data[index + 1] = c.g;
    color_data[index + 2] = c.r;
}

// Forward declaration
namespace MapNormalizer {
    ProvinceType getProvinceType(const Color&);
}

auto MapNormalizer::createProvincesFromShapeList(const PolygonList& shapes)
    -> ProvinceList
{
    ProvinceList provinces;
    provinces.reserve(shapes.size());
    for(uint32_t i = 0; i < shapes.size(); ++i) {
        auto&& shape = shapes[i];

        Color color = shape.color;

        auto prov_type = getProvinceType(color);

        provinces.push_back(Province{
            i + 1, shape.unique_color,
            prov_type,
            false,
            "unknown",
            "None",
            0,
            shape.bounding_box,
            { }
        });
    }

    return provinces;
}

