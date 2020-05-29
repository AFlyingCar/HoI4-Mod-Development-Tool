
#include "Types.h"

#include "Util.h"

/**
 * @brief Outputs the given point to the given stream.
 *
 * @param stream The stream to output into.
 * @param p The Point to output.
 *
 * @return The given stream after output.
 */
std::ostream& operator<<(std::ostream& stream, const MapNormalizer::Point2D& p)
{
    return (stream << '(' << p.x << ',' << p.y << ')');
}

/**
 * @brief Outputs the given color to the given stream.
 *
 * @param stream The stream to output into.
 * @param c The Color to output.
 * @return The given stream after output.
 */
std::ostream& operator<<(std::ostream& stream, const MapNormalizer::Color& c) {
    return (stream << std::hex << colorToRGB(c));
}

/**
 * @brief Outputs the given ProvinceType to the given stream.
 *
 * @param stream The stream to output into.
 * @param prov_type The ProvinceType to output.
 * @return The given stream after output.
 */
std::ostream& operator<<(std::ostream& stream,
                         const MapNormalizer::ProvinceType& prov_type)
{
    switch(prov_type) {
        case MapNormalizer::ProvinceType::LAND:
            return (stream << "land");
        case MapNormalizer::ProvinceType::LAKE:
            return (stream << "lake");
        case MapNormalizer::ProvinceType::SEA:
            return (stream << "sea");
        case MapNormalizer::ProvinceType::UNKNOWN:
        default:
            return (stream << "UNKNOWN{" << static_cast<int>(prov_type) << "}");
    }
}


