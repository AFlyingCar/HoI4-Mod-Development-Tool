
#include "Format.h"

auto MapNormalizer::Log::Format::operator|(const Format& other) const -> Format
{
    return Format {
        static_cast<Type>(type | other.type),
        { other.colordata[0], other.colordata[1], other.colordata[2] }
    };
}

bool MapNormalizer::Log::Format::operator==(const Format& other) const {
    return type == other.type && colordata[0] == other.colordata[0] &&
                                 colordata[1] == other.colordata[1] &&
                                 colordata[2] == other.colordata[2];
}

