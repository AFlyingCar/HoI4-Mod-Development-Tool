
#include "Types.h"

#include "Util.h"

std::ostream& operator<<(std::ostream& stream, const MapNormalizer::Color& c) {
    return (stream << std::hex << colorToRGB(c));
}

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


