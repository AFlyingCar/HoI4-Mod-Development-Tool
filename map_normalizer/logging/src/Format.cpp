
#include "Format.h"

#include <cstring>

auto MapNormalizer::Log::Format::operator|(const Format& other) const -> Format
{
    Format f { static_cast<Type>(type | other.type) };

    std::memcpy(f.data, other.data, DATA_SIZE);

    return f;
}

bool MapNormalizer::Log::Format::operator==(const Format& other) const {
    return type == other.type && std::memcmp(data, other.data, DATA_SIZE) == 0;
}

