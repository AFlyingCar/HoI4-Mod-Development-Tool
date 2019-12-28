
#include "Types.h"

#include "Util.h"

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
            return (stream << "ProvinceType{LAND}");
        case MapNormalizer::ProvinceType::LAKE:
            return (stream << "ProvinceType{LAKE}");
        case MapNormalizer::ProvinceType::SEA:
            return (stream << "ProvinceType{SEA}");
        case MapNormalizer::ProvinceType::UNKNOWN:
            return (stream << "ProvinceType{UNKNOWN}");
    }
}


